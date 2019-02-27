#include <xs1.h>
#include "xua_buffer_lite.h"
#include "rate_controller.h"
#define DEBUG_UNIT XUA_RATE_CONTROL
#define DEBUG_PRINT_ENABLE_XUA_RATE_CONTROL 1
#include "debug_print.h"

#define XUA_LIGHT_FIXED_POINT_Q_BITS        10 //Including sign bit. 10b gets us to +511.999999 to -512.000000
#define XUA_LIGHT_FIXED_POINT_TOTAL_BITS    (sizeof(xua_lite_fixed_point_t) * 8)
#define XUA_LIGHT_FIXED_POINT_FRAC_BITS     (XUA_LIGHT_FIXED_POINT_TOTAL_BITS - XUA_LIGHT_FIXED_POINT_Q_BITS)
#define XUA_LIGHT_FIXED_POINT_ONE           (1 << XUA_LIGHT_FIXED_POINT_FRAC_BITS)
#define XUA_LIGHT_FIXED_POINT_MINUS_ONE     (-XUA_LIGHT_FIXED_POINT_ONE)

#define FIFO_LEVEL_EMA_COEFF                0.939 //Proportion of signal from y[-1].
                                                  //0.939 gives ~10Hz 3db cutoff low pass filter for filter rate of 1kHz
                                                  //dsp.stackexchange.com/questions/40462/exponential-moving-average-cut-off-frequency/40465
#define FIFO_LEVEL_A_COEFF                  ((int32_t)(INT_MAX * FIFO_LEVEL_EMA_COEFF)) //Scale to signed 1.31 format
#define FIFO_LEVEL_B_COEFF                  (INT_MAX - FIFO_LEVEL_A_COEFF)

#define RANDOMISATION_PERCENT               20 //How much radnom noise to inject in percent of existing signal amplitude
#define RANDOMISATION_COEFF_A               ((INT_MAX / 100) * RANDOMISATION_PERCENT)

#define PID_CALC_OVERHEAD_BITS              2 //Allow large P,I or D constants, up to 2^(this number)


#define PID_CONTROL_P_TERM                  10.0
#define PID_CONTROL_I_TERM                  150.0
#define PID_CONTROL_D_TERM                  1.0

#define PID_RATE_MULTIPLIER                 SOF_FREQ_HZ

#define PID_CONTROL_P_TERM_COEFF            ((xua_lite_fixed_point_t)((XUA_LIGHT_FIXED_POINT_ONE >> PID_CALC_OVERHEAD_BITS) * (float)PID_CONTROL_P_TERM)) //scale to fixed point
#define PID_CONTROL_I_TERM_COEFF            ((xua_lite_fixed_point_t)((XUA_LIGHT_FIXED_POINT_ONE >> PID_CALC_OVERHEAD_BITS) * (float)PID_CONTROL_I_TERM / PID_RATE_MULTIPLIER)) //scale to fixed point
#define PID_CONTROL_D_TERM_COEFF            ((xua_lite_fixed_point_t)((XUA_LIGHT_FIXED_POINT_ONE >> PID_CALC_OVERHEAD_BITS) * (float)PID_CONTROL_D_TERM * PID_RATE_MULTIPLIER)) //scale to fixed point


static inline xua_lite_fixed_point_t do_fifo_depth_lowpass_filter(xua_lite_fixed_point_t old, int fifo_depth){
  //we grow from 32b to 64b for intermediate
  int64_t intermediate = ((int64_t)(fifo_depth << XUA_LIGHT_FIXED_POINT_FRAC_BITS) * (int64_t)FIFO_LEVEL_B_COEFF) + ((int64_t)old * (int64_t)FIFO_LEVEL_A_COEFF);
  xua_lite_fixed_point_t new_fifo_depth = (xua_lite_fixed_point_t)( intermediate >> (64 - XUA_LIGHT_FIXED_POINT_TOTAL_BITS - 1)); //-1 because signed int
  return new_fifo_depth;
}

static inline int32_t get_random_number(void)
{
  static const unsigned random_poly = 0xEDB88320;
  static unsigned random = 0x12345678;
  crc32(random, -1, random_poly);
  return (int32_t) random;
}

static inline xua_lite_fixed_point_t add_noise(xua_lite_fixed_point_t input){
  //Note the input number cannot be bigger than 2 ^ (FIXED_POINT_Q_BITS - 1) * (1 + PERCENT) else we could oveflow
  //Eg. if Q bits = 10 then biggest input value is 255.9999
  int32_t random = get_random_number();
  int32_t input_fraction = ((int64_t)input * (int64_t)RANDOMISATION_COEFF_A) >> (XUA_LIGHT_FIXED_POINT_TOTAL_BITS - 1);
  int64_t output_64 = ((int64_t)input << (XUA_LIGHT_FIXED_POINT_TOTAL_BITS - 1)) + ((int64_t)input_fraction * (int64_t)random);
  return (xua_lite_fixed_point_t)( output_64 >> (64 - XUA_LIGHT_FIXED_POINT_TOTAL_BITS - 1));
}

//Convert the control input into a pdm output (dither) with optional noise
void do_clock_nudge_pdm(xua_lite_fixed_point_t controller_out, int *clock_nudge){

  //Randomise - add a proportion of rectangular probability distribution noise to spread the spectrum
  controller_out = add_noise(controller_out);

  //Convert to pulse density modulation (sigma-delta)
  static xua_lite_fixed_point_t nudge_accumulator = 0;
  nudge_accumulator += controller_out; //Note no overflow check as if we reach XUA_LIGHT_FIXED_POINT_Q_BITS
                                       //something is very wrong
  //printf("co: %d  ratio: %f \n", controller_out, (float)controller_out/XUA_LIGHT_FIXED_POINT_ONE);
  if (nudge_accumulator >= XUA_LIGHT_FIXED_POINT_ONE){
    *clock_nudge = 1;
    nudge_accumulator -= XUA_LIGHT_FIXED_POINT_ONE;
  }
  else if (nudge_accumulator <= XUA_LIGHT_FIXED_POINT_MINUS_ONE){
    nudge_accumulator -= XUA_LIGHT_FIXED_POINT_MINUS_ONE;
    *clock_nudge = -1;
  }
  else{
    *clock_nudge = 0;
  }
}


//Do PI control and modulation for adaptive USB audio
xua_lite_fixed_point_t do_rate_control(int fill_level, pid_state_t *pid_state){

  //We always check the FIFO level after USB has produced a block, and total FIFO size is 2x max, so half full is at 3/4
  const int half_full = ((MAX_OUT_SAMPLES_PER_SOF_PERIOD * 2) * 3) / 4;
  int fill_level_wrt_half = fill_level - half_full; //Will be +ve for more than half full and negative for less than half full

  //Low pass filter fill level and get error w.r.t. to set point which is depth = 0 (relative to half full)
  xua_lite_fixed_point_t fifo_level_filtered = do_fifo_depth_lowpass_filter(pid_state->fifo_level_filtered_old , fill_level_wrt_half);
  //printf("old fill_level: %f fill_level: %f\n", (float)pid_state->fifo_level_filtered_old/(1<<XUA_LIGHT_FIXED_POINT_FRAC_BITS) , (float)fifo_level_filtered/(1<<XUA_LIGHT_FIXED_POINT_FRAC_BITS) );

  //Calculate the value for the integral term which is the accumulated fill level error
  xua_lite_fixed_point_t i_term_pre_clip = pid_state->fifo_level_accum + fifo_level_filtered;

  //clip the I term (which can wind up) to maximum fixed point representation. Check to see if overflow (which will change sign)
  if (fifo_level_filtered >= 0){ //If it was positive before, ensure it still is else clip to positive
    if (i_term_pre_clip >= pid_state->fifo_level_accum){
      //debug_printf("grow %d %d\n", (int32_t)i_term_pre_clip,  (int32_t)pid_state->fifo_level_accum);
      pid_state->fifo_level_accum = i_term_pre_clip;
    }
    else{
      pid_state->fifo_level_accum = INT_MAX;
      //debug_printf("clip+ %d\n", pid_state->fifo_level_accum);
    }
  }
  else{                           //Value was negative so ensure it still is else clip negative
    if (i_term_pre_clip <= pid_state->fifo_level_accum){
      pid_state->fifo_level_accum = i_term_pre_clip;
    }
    else{
      pid_state->fifo_level_accum = INT_MIN;
      //debug_printf("clip- %d %d\n", pid_state->fifo_level_accum, fifo_level_filtered);
    }
  }

  //Calculate D term. No clipping required because it can never be larger than the D term,
  //which is already scaled to fit within the fixed point representation
  xua_lite_fixed_point_t fifo_level_delta = fifo_level_filtered - pid_state->fifo_level_filtered_old;

  //Save to struct for next iteration
  pid_state->fifo_level_filtered_old = fifo_level_filtered;

  //Do PID calculation. Note there is an implicit cast back to xua_lite_fixed_point_t before assignment
  xua_lite_fixed_point_t p_term = (((int64_t) fifo_level_filtered * (int64_t)PID_CONTROL_P_TERM_COEFF)) >> XUA_LIGHT_FIXED_POINT_FRAC_BITS;
  xua_lite_fixed_point_t i_term = (((int64_t) pid_state->fifo_level_accum * (int64_t)PID_CONTROL_I_TERM_COEFF)) >> XUA_LIGHT_FIXED_POINT_FRAC_BITS;
  xua_lite_fixed_point_t d_term = (((int64_t) fifo_level_delta * (int64_t)PID_CONTROL_D_TERM_COEFF)) >> XUA_LIGHT_FIXED_POINT_FRAC_BITS;

  //debug_printf("p: %d i: %d f: %d\n", p_term >> XUA_LIGHT_FIXED_POINT_Q_BITS, i_term >> XUA_LIGHT_FIXED_POINT_Q_BITS, fifo_level_filtered >> (XUA_LIGHT_FIXED_POINT_FRAC_BITS - 10));
  //printf("p: %f i: %f d: %f filtered: %f integrated: %f\n", (float)p_term / (1<<(XUA_LIGHT_FIXED_POINT_FRAC_BITS-PID_CALC_OVERHEAD_BITS)), (float)i_term / (1<<(XUA_LIGHT_FIXED_POINT_FRAC_BITS-PID_CALC_OVERHEAD_BITS)), (float)d_term / (1<<(XUA_LIGHT_FIXED_POINT_FRAC_BITS-PID_CALC_OVERHEAD_BITS)), (float)fifo_level_filtered/(1<<XUA_LIGHT_FIXED_POINT_FRAC_BITS), (float)pid_state->fifo_level_accum/(1<<XUA_LIGHT_FIXED_POINT_FRAC_BITS));

  //Sum and scale to +- 1.0 (important it does not exceed these values for following step)
  xua_lite_fixed_point_t controller_out = (p_term + i_term + d_term) >> (XUA_LIGHT_FIXED_POINT_Q_BITS - 1 - PID_CALC_OVERHEAD_BITS);


  //debug_printf("filtered: %d raw: %d\n", fifo_level_filtered >> 22, fill_level_wrt_half);

  //static unsigned counter; counter++; if (counter>100){counter = 0; debug_printf("pid: %d\n",i_term >> (XUA_LIGHT_FIXED_POINT_FRAC_BITS - 10));}
  debug_printf("co: %d\n", controller_out >> XUA_LIGHT_FIXED_POINT_FRAC_BITS);
  return controller_out;
}


//Calculate feedback for asynchronous USB audio
void do_feedback_calculation(unsigned &sof_count
                                ,const unsigned mclk_hz
                                ,unsigned mclk_port_counter
                                ,unsigned &mclk_port_counter_old
                                ,long long &feedback_value
                                ,unsigned &mod_from_last_time
                                ,unsigned fb_clocks[1]){
  // Assuming 48kHz from a 24.576 master clock (0.0407uS period)
  // MCLK ticks per SOF = 125uS / 0.0407 = 3072 MCLK ticks per SOF.
  // expected Feedback is 48000/8000 = 6 samples. so 0x60000 in 16:16 format.
  // Average over 128 SOFs - 128 x 3072 = 0x60000.

  unsigned long long feedbackMul = 64ULL;
  if(AUDIO_CLASS == 1) feedbackMul = 8ULL;  // TODO Use 4 instead of 8 to avoid windows LSB issues?

  // Number of MCLK ticks in this SOF period (E.g = 125 * 24.576 = 3072)
  int mclk_ticks_this_sof_period = (int) ((short)(mclk_port_counter - mclk_port_counter_old));
  unsigned long long full_result = mclk_ticks_this_sof_period * feedbackMul * DEFAULT_FREQ;
  feedback_value += full_result;

  // Store MCLK for next time around...
  mclk_port_counter_old = mclk_port_counter;

  // Reset counts based on SOF counting.  Expect 16ms (128 HS SOFs/16 FS SOFS) per feedback poll
  // We always count 128 SOFs, so 16ms @ HS, 128ms @ FS
  if(sof_count == 128){
    //debug_printf("fb\n");
    sof_count = 0;

    feedback_value += mod_from_last_time;
    unsigned clocks = feedback_value / mclk_hz;
    mod_from_last_time = feedback_value % mclk_hz;
    feedback_value = 0;

    //Scale for working out number of samps to take from device for input
    if(AUDIO_CLASS == 2){
        clocks <<= 3;
    }
    else{
        clocks <<= 6;
    }
    asm volatile("stw %0, dp[g_speed]"::"r"(clocks));   // g_speed = clocks

    //Write to feedback EP buffer
    if (AUDIO_CLASS == 2){
        fb_clocks[0] = clocks;
    }
    else{
        fb_clocks[0] = clocks >> 2;
    }
  }
}


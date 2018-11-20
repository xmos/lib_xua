#include <stdint.h>
#include <limits.h>
#include <xs1.h>

#include "xua_commands.h"
#include "xud.h"
#include "testct_byref.h"
#define DEBUG_UNIT XUA_LITE_BUFFER
#define DEBUG_PRINT_ENABLE_XUA_LITE_BUFFER 1
#include "debug_print.h"
#include "xua.h"
#include "fifo_impl.h"
#include "xua_ep0_wrapper.h"

//Currently only single frequency supported
#define NOMINAL_SR_DEVICE                 DEFAULT_FREQ
#define NOMINAL_SR_HOST                   DEFAULT_FREQ

#define DIV_ROUND_UP(n, d) (n / d + 1)  //Always rounds up to the next integer. Needed for 48001Hz case etc.
#define BIGGEST(a, b) (a > b ? a : b)

#define SOF_FREQ_HZ (8000 - ((2 - AUDIO_CLASS) * 7000) )

//Defines for endpoint buffer sizes. Samples is total number of samples across all channels
#define MAX_OUT_SAMPLES_PER_SOF_PERIOD    (DIV_ROUND_UP(MAX_FREQ, SOF_FREQ_HZ) * NUM_USB_CHAN_OUT)
#define MAX_IN_SAMPLES_PER_SOF_PERIOD     (DIV_ROUND_UP(MAX_FREQ, SOF_FREQ_HZ) * NUM_USB_CHAN_IN)
#define MAX_OUTPUT_SLOT_SIZE              4
#define MAX_INPUT_SLOT_SIZE               4

#define OUT_AUDIO_BUFFER_SIZE_BYTES       (MAX_OUT_SAMPLES_PER_SOF_PERIOD * MAX_OUTPUT_SLOT_SIZE)
#define IN_AUDIO_BUFFER_SIZE_BYTES        (MAX_IN_SAMPLES_PER_SOF_PERIOD * MAX_INPUT_SLOT_SIZE)


static void do_feedback_calculation(unsigned &sof_count
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

#define CONTROL_LOOP 1

typedef int32_t xua_lite_fixed_point_t;
#define XUA_LIGHT_FIXED_POINT_Q_BITS        10 //Including sign bit
#define XUA_LIGHT_FIXED_POINT_FRAC_BITS     (32 - XUA_LIGHT_FIXED_POINT_Q_BITS)
#define XUA_LIGHT_FIXED_POINT_TOTAL_BITS    (XUA_LIGHT_FIXED_POINT_Q_BITS + XUA_LIGHT_FIXED_POINT_FRAC_BITS)
#define XUA_LIGHT_FIXED_POINT_ONE           (1 << XUA_LIGHT_FIXED_POINT_FRAC_BITS)
#define XUA_LIGHT_FIXED_POINT_MINUS_ONE     (-XUA_LIGHT_FIXED_POINT_ONE)

#define FIFO_LEVEL_EMA_COEFF                0.98   //Proportion of signal from y[-1]
#define FIFO_LEVEL_A_COEFF                  ((int32_t)(INT_MAX * FIFO_LEVEL_EMA_COEFF)) //Scale to signed 1.31 format
#define FIFO_LEVEL_B_COEFF                  (INT_MAX - FIFO_LEVEL_A_COEFF)

#define RANDOMISATION_PERCENT               50 //How much noise to inject in percent of existing signal amplitude
#define RANDOMISATION_COEFF_A               ((INT_MAX / 100) * RANDOMISATION_PERCENT)

#define PI_CONTROL_P_TERM                   15.0
#define PI_CONTROL_I_TERM                   1.1
#define PI_CONTROL_P_TERM_COEFF             ((xua_lite_fixed_point_t)(XUA_LIGHT_FIXED_POINT_ONE * PI_CONTROL_P_TERM)) //scale to fixed point
#define PI_CONTROL_I_TERM_COEFF             ((xua_lite_fixed_point_t)(XUA_LIGHT_FIXED_POINT_ONE * PI_CONTROL_I_TERM)) //scale to fixed point


xua_lite_fixed_point_t do_fifo_depth_lowpass_filter(xua_lite_fixed_point_t old, int fifo_depth){
  //we grow from 32b to 64b for intermediate
  int64_t intermediate = ((int64_t)(fifo_depth << XUA_LIGHT_FIXED_POINT_FRAC_BITS) * (int64_t)FIFO_LEVEL_B_COEFF) + ((int64_t)old * (int64_t)FIFO_LEVEL_A_COEFF);
  xua_lite_fixed_point_t new_fifo_depth = (xua_lite_fixed_point_t)( intermediate >> (64 - XUA_LIGHT_FIXED_POINT_TOTAL_BITS - 1)); //-1 because signed int
  return new_fifo_depth;
}

static int32_t get_random_number(void)
{
  static const unsigned random_poly = 0xEDB88320;
  static unsigned random = 0x12345678;
  crc32(random, -1, random_poly);
  return (int32_t) random;
}

static xua_lite_fixed_point_t add_noise(xua_lite_fixed_point_t input){
  //Note the input number cannot be bigger than 2 ^ (FIXED_POINT_Q_BITS - 1) * (1 + PERCENT) else we could oveflow
  //Eg. if Q bits = 10 then biggest input value is 255.9999
  int32_t random = get_random_number();
  int32_t input_fraction = ((int64_t)input * (int64_t)RANDOMISATION_COEFF_A) >> (XUA_LIGHT_FIXED_POINT_TOTAL_BITS - 1);
  int64_t output_64 = ((int64_t)input << (XUA_LIGHT_FIXED_POINT_TOTAL_BITS - 1)) + ((int64_t)input_fraction * (int64_t)random);
  return (xua_lite_fixed_point_t)( output_64 >> (64 - XUA_LIGHT_FIXED_POINT_TOTAL_BITS - 1));
}

static void fill_level_process(int fill_level, int &clock_nudge){
  //Because we always check level after USB has produced a block, and total FIFO size is 2x max, half full is at 3/4 
  const int half_full = ((MAX_OUT_SAMPLES_PER_SOF_PERIOD * 2) * 3) / 4;

#if CONTROL_LOOP
  static xua_lite_fixed_point_t fifo_level_filtered = 0;
  static xua_lite_fixed_point_t fifo_level_filtered_old = 0;
  static xua_lite_fixed_point_t fifo_level_integrated = 0;
  int fill_level_wrt_half = fill_level - half_full; //Will be +ve for more than half full and negative for less than half full

  //Do PI control
  //Low pass filter fill level and get error w.r.t. to set point which is depth = 0 (relative to half full)
  fifo_level_filtered = do_fifo_depth_lowpass_filter(fifo_level_filtered_old , fill_level_wrt_half);
  //debug_printf("o: %d n: %d\n", fifo_level_filtered_old, fifo_level_filtered);
  //Calculate integral term which is the accumulated fill level error
  xua_lite_fixed_point_t i_term_pre_clip = fifo_level_integrated + fifo_level_filtered;
  
  //clip the I term. Check to see if overflow (which will change sign)
  if (fifo_level_filtered > 0){ //If it was positive before, ensure it still is else clip to positive
    if (i_term_pre_clip > fifo_level_integrated){
      fifo_level_integrated = i_term_pre_clip;
    }
    else{
      fifo_level_integrated = INT_MAX;
      debug_printf("clip+ %d\n", fifo_level_integrated);
    }
  }
  else{                           //Value was negative so ensure it still is else clip negative
    if (i_term_pre_clip < fifo_level_integrated){
      fifo_level_integrated = i_term_pre_clip;
    }
    else{
      fifo_level_integrated = INT_MIN;
      debug_printf("clip- %d %d\n", fifo_level_integrated, fifo_level_filtered);
    }
  }

  //Do PID calculation
  xua_lite_fixed_point_t p_term = (((int64_t) fifo_level_filtered * (int64_t)PI_CONTROL_P_TERM_COEFF)) >> (64 - XUA_LIGHT_FIXED_POINT_TOTAL_BITS + 2);
  xua_lite_fixed_point_t i_term = (((int64_t) fifo_level_integrated * (int64_t)PI_CONTROL_I_TERM_COEFF)) >> (64 - XUA_LIGHT_FIXED_POINT_TOTAL_BITS + 2);
  debug_printf("p: %d i: %d f: %d\n", p_term >> XUA_LIGHT_FIXED_POINT_Q_BITS, i_term >> XUA_LIGHT_FIXED_POINT_Q_BITS, fill_level_wrt_half);

  //Sum and scale to +- 1.0 (important it does not exceed these values for following step)
  //xua_lite_fixed_point_t controller_out = (p_term + i_term) >> (XUA_LIGHT_FIXED_POINT_Q_BITS - 1);
  xua_lite_fixed_point_t controller_out = (p_term + i_term);

  controller_out = add_noise(controller_out);


  static xua_lite_fixed_point_t nudge_accumulator = 0;
  nudge_accumulator += controller_out;

  //debug_printf("na: %d -1: %d\n", nudge_accumulator, XUA_LIGHT_FIXED_POINT_MINUS_ONE);

  if (nudge_accumulator >= XUA_LIGHT_FIXED_POINT_ONE){
    clock_nudge = 1;
    nudge_accumulator -= XUA_LIGHT_FIXED_POINT_ONE;
  }
  else if (nudge_accumulator <= XUA_LIGHT_FIXED_POINT_MINUS_ONE){
    nudge_accumulator -= XUA_LIGHT_FIXED_POINT_MINUS_ONE;
    clock_nudge = -1;
  }
  else{
    clock_nudge = 0;
  }

  fifo_level_filtered_old = fifo_level_filtered;
  //debug_printf("filtered: %d raw: %d\n", fifo_level_filtered >> 22, fill_level_wrt_half);
#else
  const int trigger_high_upper = half_full + 2;
  const int trigger_low_upper = half_full - 2;

  if (fill_level >= trigger_high_upper){
    clock_nudge = 1;
    //debug_printf("Nudge down\n");
  }
  else if (fill_level <= trigger_low_upper){
    //debug_printf("Nudge up\n");
    clock_nudge = -1;
  }
  else clock_nudge = 0;
  //debug_printf("%d\n", clock_nudge);
#endif

  static unsigned counter; counter++; if (counter>SOF_FREQ_HZ){counter = 0; debug_printf("f: %d\n",fill_level);}
}


extern "C"{
void XUA_Endpoint0_lite_init(chanend c_ep0_out, chanend c_ep0_in, chanend c_audioControl,
    chanend ?c_mix_ctl, chanend ?c_clk_ctl, chanend ?c_EANativeTransport_ctrl, CLIENT_INTERFACE(i_dfu, ?dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_);
void XUA_Endpoint0_lite_loop(XUD_Result_t result, USB_SetupPacket_t sp, chanend c_ep0_out, chanend c_ep0_in, chanend c_audioControl,
    chanend ?c_mix_ctl, chanend ?c_clk_ctl, chanend ?c_EANativeTransport_ctrl, CLIENT_INTERFACE(i_dfu, ?dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_, unsigned *input_interface_num, unsigned *output_interface_num);
}
#pragma select handler
void XUD_GetSetupData_Select(chanend c, XUD_ep e_out, unsigned &length, XUD_Result_t &result);

extern XUD_ep ep0_out;
extern XUD_ep ep0_in;

//Unsafe to allow us to use fifo API without local unsafe scope
unsafe void XUA_Buffer_lite(chanend c_ep0_out, chanend c_ep0_in, chanend c_aud_out, chanend ?c_feedback, chanend c_aud_in, chanend c_sof, in port p_for_mclk_count, streaming chanend c_audio_hub) {

  debug_printf("%d\n", MAX_OUT_SAMPLES_PER_SOF_PERIOD);

  //These buffers are unions so we can access them as different types
  union buffer_aud_out{
    unsigned char bytes[OUT_AUDIO_BUFFER_SIZE_BYTES];
    short short_words[OUT_AUDIO_BUFFER_SIZE_BYTES / 2];
    long long_words[OUT_AUDIO_BUFFER_SIZE_BYTES / 4];  
  }buffer_aud_out;
  union buffer_aud_in{
    unsigned char bytes[IN_AUDIO_BUFFER_SIZE_BYTES];
    short short_words[IN_AUDIO_BUFFER_SIZE_BYTES / 2];
    unsigned long long_words[IN_AUDIO_BUFFER_SIZE_BYTES / 4];  
  }buffer_aud_in;
  
  unsigned in_subslot_size = (AUDIO_CLASS == 1) ? FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES : HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES;
  unsigned out_subslot_size = (AUDIO_CLASS == 1) ? FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES : HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES;

  //Asynch feedback calculation
  unsigned sof_count = 0;
  unsigned mclk_port_counter_old = 0;
  long long feedback_value = 0;
  unsigned mod_from_last_time = 0;
  const unsigned mclk_hz = MCLK_48;
  unsigned int fb_clocks[1] = {0}; 

  //Adapative device clock control
  int clock_nudge = 0;

  //Endpoints
  XUD_ep ep_aud_out = XUD_InitEp(c_aud_out);
  XUD_ep ep_aud_in = XUD_InitEp(c_aud_in);
  XUD_ep ep_feedback = 0;
  if (!isnull(c_feedback)) ep_feedback = XUD_InitEp(c_feedback);

  unsigned num_samples_received_from_host = 0;
  unsigned num_samples_to_send_to_host = 0;

  short samples_in_short[NUM_USB_CHAN_IN] = {0};
  short samples_out_short[NUM_USB_CHAN_OUT] = {0};

  #define c_audioControl null
  #define dfuInterface null
  XUA_Endpoint0_lite_init(c_ep0_out, c_ep0_in, c_audioControl, null, null, null, dfuInterface);
  unsigned char sbuffer[120]; //Raw buffer for EP0 data
  USB_SetupPacket_t sp;       //Parsed setup packet from EP0

  unsigned input_interface_num = 0;
  unsigned output_interface_num = 0;

  //Enable all EPs
  XUD_SetReady_OutPtr(ep_aud_out, (unsigned)buffer_aud_out.long_words);
  XUD_SetReady_InPtr(ep_aud_in, (unsigned)buffer_aud_in.long_words, num_samples_to_send_to_host);
  XUD_SetReady_Out(ep0_out, sbuffer);
  if (!isnull(c_feedback)) XUD_SetReady_InPtr(ep_feedback, (unsigned)fb_clocks, (AUDIO_CLASS == 2) ? 4 : 3);
 

  //Send initial samples so audiohub is not blocked
  for (int i = 0; i < 2 * (NUM_USB_CHAN_OUT + (XUA_ADAPTIVE != 0 ? 1 : 0)); i++) c_audio_hub <: 0;

  //FIFOs from EP buffers to audio
  short host_to_device_fifo_storage[MAX_OUT_SAMPLES_PER_SOF_PERIOD * 2];
  short device_to_host_fifo_storage[MAX_IN_SAMPLES_PER_SOF_PERIOD * 2];
  mem_fifo_short_t host_to_device_fifo = {sizeof(host_to_device_fifo_storage)/sizeof(host_to_device_fifo_storage[0]), host_to_device_fifo_storage, 0, 0};
  mem_fifo_short_t device_to_host_fifo = {sizeof(device_to_host_fifo_storage)/sizeof(device_to_host_fifo_storage[0]), device_to_host_fifo_storage, 0, 0};
  volatile mem_fifo_short_t * unsafe host_to_device_fifo_ptr = &host_to_device_fifo;
  volatile mem_fifo_short_t * unsafe device_to_host_fifo_ptr = &device_to_host_fifo;

  //XUD transaction variables passed in by reference
  XUD_Result_t result;
  unsigned length = 0;
  unsigned u_tmp; //For select channel input by ref on EP0
  int s_tmp;      //For select on channel from audiohub
  while(1){
    #pragma ordered
    select{
      //Handle EP0 requests
      case XUD_GetSetupData_Select(c_ep0_out, ep0_out, length, result):
      timer tmr; int t0, t1; tmr :> t0;

        debug_printf("ep0, result: %d, length: %d\n", result, length); //-1 reset, 0 ok, 1 error
        USB_ParseSetupPacket(sbuffer, sp); //Parse data buffer end populate SetupPacket struct
        
        XUA_Endpoint0_lite_loop(result, sp, c_ep0_out, c_ep0_in, c_audioControl, null/*mix*/, null/*clk*/, null/*EA*/, dfuInterface, &input_interface_num, &output_interface_num);
        XUD_SetReady_Out(ep0_out, sbuffer);
        tmr :> t1; debug_printf("c%d\n", t1 - t0);

      break;

      //SOF handling
      case inuint_byref(c_sof, u_tmp):
      timer tmr; int t0, t1; tmr :> t0;
        unsigned mclk_port_counter = 0;
        asm volatile(" getts %0, res[%1]" : "=r" (mclk_port_counter) : "r" (p_for_mclk_count));
        if (!isnull(c_feedback)) do_feedback_calculation(sof_count, mclk_hz, mclk_port_counter, mclk_port_counter_old, feedback_value, mod_from_last_time, fb_clocks);
        sof_count++;
        tmr :> t1; debug_printf("s%d\n", t1 - t0);

      break;

      //Receive samples from host
      case XUD_GetData_Select(c_aud_out, ep_aud_out, length, result):
      timer tmr; int t0, t1; tmr :> t0;

        num_samples_received_from_host = length / out_subslot_size;
  
        fifo_ret_t ret = fifo_block_push_short(host_to_device_fifo_ptr, buffer_aud_out.short_words, num_samples_received_from_host);
        if (ret != FIFO_SUCCESS) debug_printf("h2d full\n");
        num_samples_to_send_to_host = num_samples_received_from_host;
        
        int fill_level = fifo_get_fill_short(host_to_device_fifo_ptr);
        fill_level_process(fill_level, clock_nudge);

        //Mark EP as ready for next frame from host
        XUD_SetReady_OutPtr(ep_aud_out, (unsigned)buffer_aud_out.long_words);
        tmr :> t1; debug_printf("o%d\n", t1 - t0);
      break;

      //Send asynch explicit feedback value, but only if enabled
      case !isnull(c_feedback) => XUD_SetData_Select(c_feedback, ep_feedback, result):
      timer tmr; int t0, t1; tmr :> t0;

        XUD_SetReady_In(ep_feedback, (fb_clocks, unsigned char[]), (AUDIO_CLASS == 2) ? 4 : 3);
        //debug_printf("0x%x\n", fb_clocks[0]);
        tmr :> t1; debug_printf("f%d\n", t1 - t0);

      break;

      //Send samples to host
      case XUD_SetData_Select(c_aud_in, ep_aud_in, result):
      timer tmr; int t0, t1; tmr :> t0;

        if (output_interface_num == 0) num_samples_to_send_to_host = (DEFAULT_FREQ  / SOF_FREQ_HZ) * NUM_USB_CHAN_IN;

        fifo_ret_t ret = fifo_block_pop_short(device_to_host_fifo_ptr, buffer_aud_in.short_words, num_samples_received_from_host);
        if (ret != FIFO_SUCCESS) debug_printf("d2h empty\n");

        //Populate the input buffer ready for the next read
        //pack_samples_to_buff(loopback_samples, num_samples_to_send_to_host, in_subslot_size, buffer_aud_in);
        //Use the number of samples we received last time so we are always balanced (assumes same in/out count)
    
        unsigned input_buffer_size = num_samples_to_send_to_host * in_subslot_size;
        XUD_SetReady_InPtr(ep_aud_in, (unsigned)buffer_aud_in.long_words, input_buffer_size); //loopback
        num_samples_to_send_to_host = 0;
        tmr :> t1; debug_printf("i%d\n", t1 - t0);

      break;

      //Exchange samples with audiohub. Note we are using channel buffering here to act as a FIFO
      case c_audio_hub :> s_tmp:
      timer tmr; int t0, t1; tmr :> t0;
        samples_in_short[0] = s_tmp >> 16;
        for (int i = 1; i < NUM_USB_CHAN_IN; i++){
          c_audio_hub :> s_tmp;
          samples_in_short[i] = s_tmp >> 16;
        }
        fifo_ret_t ret = fifo_block_pop_short(host_to_device_fifo_ptr, samples_out_short, NUM_USB_CHAN_OUT);
        if (ret != FIFO_SUCCESS && output_interface_num != 0) debug_printf("h2d empty\n");
        for (int i = 0; i < NUM_USB_CHAN_OUT; i++) c_audio_hub <: (int)samples_out_short[i] << 16;
        if (XUA_ADAPTIVE) c_audio_hub <: clock_nudge;
        ret = fifo_block_push_short(device_to_host_fifo_ptr, samples_in_short, NUM_USB_CHAN_IN);
        if (ret != FIFO_SUCCESS && input_interface_num != 0) debug_printf("d2h full\n");
        tmr :> t1; debug_printf("a%d\n", t1 - t0);
      break;
    }
  }
}
extern port p_sda;

[[combinable]]
//Unsafe to allow us to use fifo API without local unsafe scope
unsafe void XUA_Buffer_lite2(server ep0_control_if i_ep0_ctl, chanend c_aud_out, chanend ?c_feedback, chanend c_aud_in, chanend c_sof, in port p_for_mclk_count, streaming chanend c_audio_hub) {

  debug_printf("%d\n", MAX_OUT_SAMPLES_PER_SOF_PERIOD);

  //These buffers are unions so we can access them as different types
  union buffer_aud_out{
    unsigned char bytes[OUT_AUDIO_BUFFER_SIZE_BYTES];
    short short_words[OUT_AUDIO_BUFFER_SIZE_BYTES / 2];
    long long_words[OUT_AUDIO_BUFFER_SIZE_BYTES / 4];  
  }buffer_aud_out;
  union buffer_aud_in{
    unsigned char bytes[IN_AUDIO_BUFFER_SIZE_BYTES];
    short short_words[IN_AUDIO_BUFFER_SIZE_BYTES / 2];
    unsigned long long_words[IN_AUDIO_BUFFER_SIZE_BYTES / 4];  
  }buffer_aud_in;
  
  unsigned in_subslot_size = (AUDIO_CLASS == 1) ? FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES : HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES;
  unsigned out_subslot_size = (AUDIO_CLASS == 1) ? FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES : HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES;

  //Asynch feedback calculation
  unsigned sof_count = 0;
  unsigned mclk_port_counter_old = 0;
  long long feedback_value = 0;
  unsigned mod_from_last_time = 0;
  const unsigned mclk_hz = MCLK_48;
  unsigned int fb_clocks[1] = {0}; 

  //Adapative device clock control
  int clock_nudge = 0;

  //Endpoints
  XUD_ep ep_aud_out = XUD_InitEp(c_aud_out);
  XUD_ep ep_aud_in = XUD_InitEp(c_aud_in);
  XUD_ep ep_feedback = 0;
  if (!isnull(c_feedback)) ep_feedback = XUD_InitEp(c_feedback);

  unsigned num_samples_received_from_host = 0;
  unsigned num_samples_to_send_to_host = 0;

  unsigned input_interface_num = 0;
  unsigned output_interface_num = 0;

  //Enable all EPs
  XUD_SetReady_OutPtr(ep_aud_out, (unsigned)buffer_aud_out.long_words);
  XUD_SetReady_InPtr(ep_aud_in, (unsigned)buffer_aud_in.long_words, num_samples_to_send_to_host);
  if (!isnull(c_feedback)) XUD_SetReady_InPtr(ep_feedback, (unsigned)fb_clocks, (AUDIO_CLASS == 2) ? 4 : 3);
 
  short samples_in_short[NUM_USB_CHAN_IN] = {0};
  short samples_out_short[NUM_USB_CHAN_OUT] = {0};


  //Send initial samples so audiohub is not blocked
  const unsigned n_sample_periods_to_preload = 2;
  for (int i = 0; i < n_sample_periods_to_preload * (NUM_USB_CHAN_OUT + (XUA_ADAPTIVE != 0 ? 1 : 0)); i++) c_audio_hub <: 0;

  //FIFOs from EP buffers to audio
  short host_to_device_fifo_storage[MAX_OUT_SAMPLES_PER_SOF_PERIOD * 2];
  short device_to_host_fifo_storage[MAX_IN_SAMPLES_PER_SOF_PERIOD * 2];
  mem_fifo_short_t host_to_device_fifo = {sizeof(host_to_device_fifo_storage)/sizeof(host_to_device_fifo_storage[0]), host_to_device_fifo_storage, 0, 0};
  mem_fifo_short_t device_to_host_fifo = {sizeof(device_to_host_fifo_storage)/sizeof(device_to_host_fifo_storage[0]), device_to_host_fifo_storage, 0, 0};
  volatile mem_fifo_short_t * unsafe host_to_device_fifo_ptr = &host_to_device_fifo;
  volatile mem_fifo_short_t * unsafe device_to_host_fifo_ptr = &device_to_host_fifo;

  //XUD transaction variables passed in by reference
  XUD_Result_t result;
  unsigned length = 0;
  unsigned u_tmp; //For select channel input by ref on EP0
  int s_tmp;      //For select on channel from audiohub
  while(1){
    select{
      //Handle EP0 requests
      case i_ep0_ctl.set_output_interface(unsigned num):
        output_interface_num = num;
        debug_printf("output_interface_num: %d\n", num);
      break;

      case i_ep0_ctl.set_input_interface(unsigned num):
        input_interface_num = num;
        debug_printf("input_interface_num: %d\n", num);
      break;

      case i_ep0_ctl.set_host_active(unsigned active):
      break;

      //SOF handling
      case inuint_byref(c_sof, u_tmp):
      timer tmr; int t0, t1; tmr :> t0;
        unsigned mclk_port_counter = 0;
        asm volatile(" getts %0, res[%1]" : "=r" (mclk_port_counter) : "r" (p_for_mclk_count));
        if (!isnull(c_feedback)) do_feedback_calculation(sof_count, mclk_hz, mclk_port_counter, mclk_port_counter_old, feedback_value, mod_from_last_time, fb_clocks);
        sof_count++;
        //tmr :> t1; debug_printf("s%d\n", t1 - t0);
        uint16_t port_counter;
        p_sda <: 1 @ port_counter;
        p_sda @ port_counter + 100 <: 0;
      break;

      //Receive samples from host
      case XUD_GetData_Select(c_aud_out, ep_aud_out, length, result):
      timer tmr; int t0, t1; tmr :> t0;

        num_samples_received_from_host = length / out_subslot_size;

        if (num_samples_received_from_host != 96) debug_printf("hs: %d\n", num_samples_received_from_host);
  
        fifo_ret_t ret = fifo_block_push_short_fast(host_to_device_fifo_ptr, buffer_aud_out.short_words, num_samples_received_from_host);
        if (ret != FIFO_SUCCESS) debug_printf("h2d full\n");
        num_samples_to_send_to_host = num_samples_received_from_host;
        
        int fill_level = fifo_get_fill_short(host_to_device_fifo_ptr);
        fill_level_process(fill_level, clock_nudge);

        //Mark EP as ready for next frame from host
        XUD_SetReady_OutPtr(ep_aud_out, (unsigned)buffer_aud_out.long_words);
        //tmr :> t1; debug_printf("o%d\n", t1 - t0);
      break;

      //Send asynch explicit feedback value, but only if enabled
      case !isnull(c_feedback) => XUD_SetData_Select(c_feedback, ep_feedback, result):
      timer tmr; int t0, t1; tmr :> t0;

        XUD_SetReady_In(ep_feedback, (fb_clocks, unsigned char[]), (AUDIO_CLASS == 2) ? 4 : 3);
        //debug_printf("0x%x\n", fb_clocks[0]);
        //tmr :> t1; debug_printf("f%d\n", t1 - t0);
      break;

      //Send samples to host
      case XUD_SetData_Select(c_aud_in, ep_aud_in, result):
      timer tmr; int t0, t1; tmr :> t0;

        //If host is not streaming out, then send a fixed number of samples to host
        if (output_interface_num == 0) num_samples_to_send_to_host = (DEFAULT_FREQ  / SOF_FREQ_HZ) * NUM_USB_CHAN_IN;

        fifo_ret_t ret = fifo_block_pop_short_fast(device_to_host_fifo_ptr, buffer_aud_in.short_words, num_samples_received_from_host);
        if (ret != FIFO_SUCCESS) {
          memset(buffer_aud_in.short_words, 0, sizeof(buffer_aud_in.short_words));
          debug_printf("d2h empty\n");
        }

        //Populate the input buffer ready for the next read
        //pack_samples_to_buff(loopback_samples, num_samples_to_send_to_host, in_subslot_size, buffer_aud_in);
        //Use the number of samples we received last time so we are always balanced (assumes same in/out count)
    
        unsigned input_buffer_size = num_samples_to_send_to_host * in_subslot_size;
        XUD_SetReady_InPtr(ep_aud_in, (unsigned)buffer_aud_in.long_words, input_buffer_size); //loopback
        num_samples_to_send_to_host = 0;
        //tmr :> t1; debug_printf("i%d\n", t1 - t0);
      break;

      //Exchange samples with audiohub. Note we are using channel buffering here to act as a FIFO
      case c_audio_hub :> s_tmp:
      timer tmr; int t0, t1; tmr :> t0;
        samples_in_short[0] = s_tmp >> 16;
        for (int i = 1; i < NUM_USB_CHAN_IN; i++){
          c_audio_hub :> s_tmp;
          samples_in_short[i] = s_tmp >> 16;
        }
        fifo_ret_t ret = fifo_block_pop_short(host_to_device_fifo_ptr, samples_out_short, NUM_USB_CHAN_OUT);
        if (ret != FIFO_SUCCESS && output_interface_num != 0) {
          memset(samples_out_short, 0, sizeof(samples_out_short));
          debug_printf("h2d empty\n");
        }
        for (int i = 0; i < NUM_USB_CHAN_OUT; i++) c_audio_hub <: (int)samples_out_short[i] << 16;
        if (XUA_ADAPTIVE) c_audio_hub <: clock_nudge;
        ret = fifo_block_push_short(device_to_host_fifo_ptr, samples_in_short, NUM_USB_CHAN_IN);
        if (ret != FIFO_SUCCESS && input_interface_num != 0) debug_printf("d2h full\n");
        //tmr :> t1; debug_printf("a%d\n", t1 - t0);
      break;
    }
  }
}
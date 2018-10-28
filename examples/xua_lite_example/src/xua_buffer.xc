#include <stdint.h>

#include <xs1.h>

#include "xua_commands.h"
#include "xud.h"
#include "testct_byref.h"
#define DEBUG_UNIT XUA_LITE_BUFFER
#define DEBUG_PRINT_ENABLE_XUA_LITE_BUFFER 1
#include "debug_print.h"
#include "xua.h"
#include "fifo_impl.h"

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

//Helper to disassemble USB packets into 32b left aligned audio samples
static inline void unpack_buff_to_samples(unsigned char input[], const unsigned n_samples, const unsigned slot_size, int output[]){
  switch(slot_size){
    case 4:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 4;
        output[i] = (input[base + 3] << 24) | (input[base + 2] << 16) | (input[base + 1] << 8) | input[base + 0];
      }
    break;
    case 3:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 3;
        output[i] = (input[base + 2] << 24) | (input[base + 1] << 16) | (input[base + 0] << 8);
      }
    break;
    case 2:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 2;
        output[i] = (input[base + 1] << 24) | (input[base + 0] << 16);
      }
    break;
    default:
      debug_printf("Invalid slot_size\n");
    break;
  }
}

//Helper to assemble USB packets from 32b left aligned audio samples
static inline void pack_samples_to_buff(int input[], const unsigned n_samples, const unsigned slot_size, unsigned char output[]){
  switch(slot_size){
    case 4:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 4;
        unsigned in_word = (unsigned)input[i];
        output[base + 0] = in_word & 0xff;
        output[base + 1] = (in_word & 0xff00) >> 8;
        output[base + 2] = (in_word & 0xff0000) >> 16;
        output[base + 3] = (in_word) >> 24;
      }
    break;
    case 3:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 3;
        unsigned in_word = (unsigned)input[i];
        output[base + 0] = (in_word & 0xff00) >> 8;
        output[base + 1] = (in_word & 0xff0000) >> 16;
        output[base + 2] = (in_word) >> 24;
      }
    break;
    case 2:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 2;
        unsigned in_word = (unsigned)input[i];
        output[base + 0] = (in_word & 0xff0000) >> 16;
        output[base + 1] = (in_word) >> 24;
      }
    break;
    default:
      debug_printf("Invalid slot_size\n");
    break;
  }
}


void XUA_Buffer_lite(chanend c_aud_out, chanend c_feedback, chanend c_aud_in, chanend c_sof, chanend c_aud_ctl, in port p_for_mclk_count, chanend c_audio_hub) {

  debug_printf("%d\n", MAX_OUT_SAMPLES_PER_SOF_PERIOD);

  unsigned sampleFreq = DEFAULT_FREQ;

  unsigned char buffer_aud_out[OUT_AUDIO_BUFFER_SIZE_BYTES];
  unsigned char buffer_aud_in[IN_AUDIO_BUFFER_SIZE_BYTES];
  
  #define FEEDBACK_BUFF_SIZE    4
  unsigned char buffer_feedback[FEEDBACK_BUFF_SIZE];
  unsigned int fb_clocks[1] = {0}; 


  unsigned in_subslot_size = (AUDIO_CLASS == 1) ? FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES : HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES;
  unsigned out_subslot_size = (AUDIO_CLASS == 1) ? FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES : HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES;

  unsigned in_num_chan = NUM_USB_CHAN_IN;
  unsigned out_num_chan = NUM_USB_CHAN_OUT;

  unsigned tmp;

  unsigned lastClock = 0;
  unsigned clocks = 0;
  long long clockcounter = 0;
  unsigned sof_count=0;
  unsigned mod_from_last_time = 0;

  
  XUD_ep ep_aud_out = XUD_InitEp(c_aud_out);
  XUD_ep ep_feedback = XUD_InitEp(c_feedback);
  XUD_ep ep_aud_in = XUD_InitEp(c_aud_in);

  unsigned num_samples_received_from_host = 0;
  unsigned outstanding_samples_to_host = 0;
  unsigned num_samples_to_send_to_host = 0;

  XUD_SetReady_OutPtr(ep_aud_out, (unsigned)buffer_aud_out);
  XUD_SetReady_InPtr(ep_aud_in, (unsigned)buffer_aud_in, num_samples_to_send_to_host);
  XUD_SetReady_InPtr(ep_feedback, (unsigned)buffer_feedback, FEEDBACK_BUFF_SIZE);

  int loopback_samples[MAX_OUT_SAMPLES_PER_SOF_PERIOD] = {0};
  int32_t samples_out[NUM_USB_CHAN_OUT] = {0};
  int32_t samples_in[NUM_USB_CHAN_IN] = {0};

  unsafe{

    int host_to_device_fifo_storage[MAX_OUT_SAMPLES_PER_SOF_PERIOD * 2];
    mem_fifo_t host_to_device_fifo = {sizeof(host_to_device_fifo_storage)/sizeof(host_to_device_fifo_storage[0])
                                            , host_to_device_fifo_storage, 0, 0};
    volatile mem_fifo_t * unsafe host_to_device_fifo_ptr = &host_to_device_fifo;

    XUD_Result_t result;
    unsigned length = 0;

    while(1){
      select{
        //Handle control path from EP0
        case testct_byref(c_aud_ctl, tmp):
          //ignore tmp as is used for reboot signalling only
          unsigned cmd = inuint(c_aud_ctl);
          
          debug_printf("c_aud_ctl cmd: %d\n", cmd);
          if(cmd == SET_SAMPLE_FREQ){
              unsigned receivedSampleFreq = inuint(c_aud_ctl);
              debug_printf("SET_SAMPLE_FREQ: %d\n", receivedSampleFreq);
              sampleFreq = receivedSampleFreq;
          }

          else if(cmd == SET_STREAM_FORMAT_IN){
            unsigned formatChange_DataFormat = inuint(c_aud_ctl);
            unsigned formatChange_NumChans = inuint(c_aud_ctl);
            unsigned formatChange_SubSlot = inuint(c_aud_ctl);
            unsigned formatChange_SampRes = inuint(c_aud_ctl);
            debug_printf("SET_STREAM_FORMAT_IN: %d %d %d %d\n", formatChange_DataFormat, formatChange_NumChans, formatChange_SubSlot, formatChange_SampRes);
            in_subslot_size = formatChange_SubSlot;
            in_num_chan = formatChange_NumChans;
          }

          else if (cmd == SET_STREAM_FORMAT_OUT)
          {
              XUD_BusSpeed_t busSpeed;
              unsigned formatChange_DataFormat = inuint(c_aud_ctl);
              unsigned formatChange_NumChans = inuint(c_aud_ctl);
              unsigned formatChange_SubSlot = inuint(c_aud_ctl);
              unsigned formatChange_SampRes = inuint(c_aud_ctl);
              debug_printf("SET_STREAM_FORMAT_OUT: %d %d %d %d\n", formatChange_DataFormat, formatChange_NumChans, formatChange_SubSlot, formatChange_SampRes);
              out_subslot_size = formatChange_SubSlot;
              out_num_chan = formatChange_NumChans;
          }

          else{
            debug_printf("Unhandled command\n");
          }
          outct(c_aud_ctl, XS1_CT_END);
        break;

        //SOF
        case inuint_byref(c_sof, tmp):
          unsigned mclk_port_count = 0;
          asm volatile(" getts %0, res[%1]" : "=r" (mclk_port_count) : "r" (p_for_mclk_count));

          /* Assuming 48kHz from a 24.576 master clock (0.0407uS period)
           * MCLK ticks per SOF = 125uS / 0.0407 = 3072 MCLK ticks per SOF.
           * expected Feedback is 48000/8000 = 6 samples. so 0x60000 in 16:16 format.
           * Average over 128 SOFs - 128 x 3072 = 0x60000.
           */

          unsigned long long feedbackMul = 64ULL;
          if(AUDIO_CLASS == 1)
              feedbackMul = 8ULL;  /* TODO Use 4 instead of 8 to avoid windows LSB issues? */

          /* Number of MCLK ticks in this SOF period (E.g = 125 * 24.576 = 3072) */
          int count = (int) ((short)(mclk_port_count - lastClock));

          unsigned long long full_result = count * feedbackMul * DEFAULT_FREQ;

          clockcounter += full_result;

          /* Store MCLK for next time around... */
          lastClock = mclk_port_count;

          /* Reset counts based on SOF counting.  Expect 16ms (128 HS SOFs/16 FS SOFS) per feedback poll
           * We always count 128 SOFs, so 16ms @ HS, 128ms @ FS */
          if(sof_count == 128)
          {
              //debug_printf("fb\n");
              sof_count = 0;

              clockcounter += mod_from_last_time;
              clocks = clockcounter / MCLK_48;
              mod_from_last_time = clockcounter % MCLK_48;

              //Scale for working out number of samps to take from device for input
              if(AUDIO_CLASS == 2)
              {
                  clocks <<= 3;
              }
              else
              {
                  clocks <<= 6;
              }
              asm volatile("stw %0, dp[g_speed]"::"r"(clocks));   // g_speed = clocks

              //Write to feedback EP buffer
              if (AUDIO_CLASS == 2)
              {
                  fb_clocks[0] = clocks;
              }
              else
              {
                  fb_clocks[0] = clocks >> 2;
              }
              clockcounter = 0;
          }
          sof_count++;
        break;

        //Receive samples from host
        case XUD_GetData_Select(c_aud_out, ep_aud_out, length, result):
          num_samples_received_from_host = length / out_subslot_size;
          //debug_printf("out samps: %d\n", num_samples_received_from_host);
          outstanding_samples_to_host += num_samples_received_from_host;

          unpack_buff_to_samples(buffer_aud_out, num_samples_received_from_host, out_subslot_size, loopback_samples);

          fifo_ret_t ret = fifo_block_push(host_to_device_fifo_ptr, loopback_samples, num_samples_received_from_host);
          if (ret != FIFO_SUCCESS) debug_printf("full\n");

          num_samples_to_send_to_host = num_samples_received_from_host;
          
          //Mark EP as ready for next frame from host
          XUD_SetReady_OutPtr(ep_aud_out, (unsigned)buffer_aud_out);
        break;

        //Send feedback
        case XUD_SetData_Select(c_feedback, ep_feedback, result):
          //debug_printf("ep_feedback\n");
          //XUD_SetReady_InPtr(ep_feedback, (unsigned)buffer_feedback, FEEDBACK_BUFF_SIZE);
          if (AUDIO_CLASS == 2)
          {
              XUD_SetReady_In(ep_feedback, (fb_clocks, unsigned char[]), 4);
          }
          else
          {
              XUD_SetReady_In(ep_feedback, (fb_clocks, unsigned char[]), 3);
          }

        break;

        //Send samples to host
        case XUD_SetData_Select(c_aud_in, ep_aud_in, result):
          //debug_printf("sent data\n");

          //Populate the input buffer ready for the next read
          pack_samples_to_buff(loopback_samples, num_samples_to_send_to_host, in_subslot_size, buffer_aud_in);
          //Use the number of samples we received last time so we are always balanced (assumes same in/out count)
          
          unsigned input_buffer_size = num_samples_to_send_to_host * in_subslot_size;
          XUD_SetReady_InPtr(ep_aud_in, (unsigned)buffer_aud_in, input_buffer_size); //loopback
          num_samples_to_send_to_host = 0;
        break;

        case c_audio_hub :> samples_in[0]:
          for (int i = 1; i < NUM_USB_CHAN_IN; i++) c_audio_hub :> samples_in[i];
          // for (int i = 0; i < NUM_USB_CHAN_OUT; i++) c_audio_hub <: samples_out[1];
          int out_samps[NUM_USB_CHAN_OUT];
          fifo_ret_t ret = fifo_block_pop(host_to_device_fifo_ptr, out_samps, NUM_USB_CHAN_OUT);
          //if (ret != FIFO_SUCCESS) debug_printf("empty\n");
          for (int i = 0; i < NUM_USB_CHAN_OUT; i++) c_audio_hub <: out_samps[i];
        break;
      }
    }
  }
}
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
#include "rate_controller.h"
#include "xua_buffer_lite.h"


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
  pid_state_t pid_state = {0, 0};

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
        if (isnull(c_feedback))  do_clock_nudge_pdm(do_rate_control(fill_level, &pid_state), &clock_nudge);

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
  pid_state_t pid_state = {0, 0};


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
        //Reset output FIFO if moving from idle to streaming
        if (num != 0 && output_interface_num == 0) fifo_init_short(host_to_device_fifo_ptr);
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
        if (isnull(c_feedback))  do_clock_nudge_pdm(do_rate_control(fill_level, &pid_state), &clock_nudge);

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
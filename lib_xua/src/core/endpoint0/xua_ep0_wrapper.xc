#include <xs1.h>
#include <safestring.h>
#include <stddef.h>
#include "xua.h"

#define DEBUG_UNIT EP0_WRAPPER
#define DEBUG_PRINT_ENABLE_EP0_WRAPPER 1
#include "debug_print.h"

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


void XUA_Endpoint0_select(chanend c_ep0_out, chanend c_ep0_in, chanend c_audioControl,
    chanend ?c_mix_ctl, chanend ?c_clk_ctl, chanend ?c_EANativeTransport_ctrl, CLIENT_INTERFACE(i_dfu, dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_)
{

    USB_SetupPacket_t sp;
    XUA_Endpoint0_lite_init(c_ep0_out, c_ep0_in, c_audioControl, c_mix_ctl, c_clk_ctl, c_EANativeTransport_ctrl, dfuInterface);
    unsigned char sbuffer[120];
    XUD_SetReady_Out(ep0_out, sbuffer);

    unsigned input_interface_num = 0;
    unsigned output_interface_num = 0;


    while(1){

      XUD_Result_t result = XUD_RES_ERR;
      unsigned length = 0;

      //XUD_Result_t result = XUD_GetSetupBuffer(ep0_out, sbuffer, &length); //Flattened from xud_device
      // result = XUD_GetSetupData(ep0_out, sbuffer, length);//Flattened from XUD_EpFunctions.xc

      select{
        case XUD_GetSetupData_Select(c_ep0_out, ep0_out, length, result):
        break;
      }

      if (result == XUD_RES_OKAY)
      {
          /* Parse data buffer end populate SetupPacket struct */
          USB_ParseSetupPacket(sbuffer, sp);
      }
      debug_printf("ep0, result: %d, length: %d\n", result, length); //-1 reset, 0 ok, 1 error

      XUA_Endpoint0_lite_loop(result, sp, c_ep0_out, c_ep0_in, c_audioControl, c_mix_ctl, c_clk_ctl, c_EANativeTransport_ctrl, dfuInterface, &input_interface_num, &output_interface_num);
      XUD_SetReady_Out(ep0_out, sbuffer);

    }
}
#include <xs1.h>
#include <safestring.h>
#include <stddef.h>
#include "xua.h"
#include "xua_ep0_wrapper.h"

#define DEBUG_UNIT EP0_WRAPPER
#define DEBUG_PRINT_ENABLE_EP0_WRAPPER 0
#include "debug_print.h"

extern XUD_ep ep0_out;
extern XUD_ep ep0_in;

[[combinable]]
void XUA_Endpoint0_select(chanend c_ep0_out, chanend c_ep0_in, client ep0_control_if i_ep0_ctl,  CLIENT_INTERFACE(i_dfu, ?dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_)
{

  USB_SetupPacket_t sp;
  XUA_Endpoint0_lite_init(c_ep0_out, c_ep0_in, null, null, null, null, dfuInterface VENDOR_REQUESTS_PARAMS_);
  unsigned char sbuffer[120];
  XUD_SetReady_Out(ep0_out, sbuffer);

  unsigned input_interface_num = 0;
  unsigned output_interface_num = 0;

  XUD_Result_t result = XUD_RES_ERR;
  unsigned length = 0;

  while(1){
    select{
      case XUD_GetSetupData_Select(c_ep0_out, ep0_out, length, result):
        if (result == XUD_RES_OKAY)
        {
            /* Parse data buffer end populate SetupPacket struct */
            USB_ParseSetupPacket(sbuffer, sp);
        }
        debug_printf("ep0, result: %d, length: %d\n", result, length); //-1 reset, 0 ok, 1 error

        XUA_Endpoint0_lite_loop(result, sp, c_ep0_out, c_ep0_in, null, null, null, null, dfuInterface
        VENDOR_REQUESTS_PARAMS_, &input_interface_num, &output_interface_num);
        i_ep0_ctl.set_output_interface(output_interface_num);
        i_ep0_ctl.set_input_interface(input_interface_num);

        XUD_SetReady_Out(ep0_out, sbuffer);
      break;
    }
  }
}

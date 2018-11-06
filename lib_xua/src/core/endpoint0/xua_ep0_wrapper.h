#ifndef _EP0_WRAPPER_
#define _EP0_WRAPPER_

#include <xs1.h>
#include <safestring.h>
#include <stddef.h>
#include "xua.h"

typedef interface ep0_control_if{
  void set_output_interface(unsigned num);
  void set_input_interface(unsigned num);
  void set_host_active(unsigned num);
}ep0_control_if;

extern "C"{
void XUA_Endpoint0_lite_init(chanend c_ep0_out, chanend c_ep0_in, chanend c_audioControl,
    chanend ?c_mix_ctl, chanend ?c_clk_ctl, chanend ?c_EANativeTransport_ctrl, CLIENT_INTERFACE(i_dfu, ?dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_);
void XUA_Endpoint0_lite_loop(XUD_Result_t result, USB_SetupPacket_t sp, chanend c_ep0_out, chanend c_ep0_in, chanend c_audioControl,
    chanend ?c_mix_ctl, chanend ?c_clk_ctl, chanend ?c_EANativeTransport_ctrl, CLIENT_INTERFACE(i_dfu, ?dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_, unsigned *input_interface_num, unsigned *output_interface_num);
}
#pragma select handler
void XUD_GetSetupData_Select(chanend c, XUD_ep e_out, unsigned &length, XUD_Result_t &result);

[[combinable]]
void XUA_Endpoint0_select(chanend c_ep0_out, chanend c_ep0_in, client ep0_control_if i_ep0_ctl,  CLIENT_INTERFACE(i_dfu, ?dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_);

#endif
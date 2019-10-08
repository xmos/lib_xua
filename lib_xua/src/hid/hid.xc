#include <xs1.h>
#include "xud.h"
#include "hid.h"
#include "xud_std_requests.h"
#include "xua_hid.h"

static unsigned hidSetIdle = 0;

unsigned HidIsSetIdleSilenced(void)
{
  return hidSetIdle;
}

XUD_Result_t HidInterfaceClassRequests(XUD_ep c_ep0_out, XUD_ep c_ep0_in,
                                       USB_SetupPacket_t &sp)
{
    switch (sp.bRequest) {
      case HID_SET_IDLE:
          printstrln("HID_SET_IDLE\n");
          hidSetIdle = 1; // TODO implement duration
          return XUD_DoSetRequestStatus(c_ep0_in);
      default:
          break;
    }
    return XUD_RES_ERR;
}

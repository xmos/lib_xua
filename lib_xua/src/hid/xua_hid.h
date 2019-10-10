#include <xs1.h>
#include <xccompat.h>
#include "xud.h"
#include "xud_std_requests.h"

void HidCalcNextReportTime( void );

void HidCaptureReportTime( void );

XUD_Result_t HidInterfaceClassRequests(
  XUD_ep c_ep0_out,
  XUD_ep c_ep0_in,
  REFERENCE_PARAM( USB_SetupPacket_t, sp ));

unsigned HidIsSetIdleSilenced( void );

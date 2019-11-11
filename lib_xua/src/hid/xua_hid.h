// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <xccompat.h>
#include "xud.h"
#include "xud_std_requests.h"

/**
 *  \brief Calculate the next time to respond with a HID Report.
 *
 *  If the USB Host has previously sent a valid HID Set_Idle request with
 *    a duration of zero or greater than the default reporting interval,
 *    the device sends HID Reports periodically or when the value of the
 *    payload has changed.
 *
 *  This function calculates the time for sending the next periodic
 *    HID Report.
 */
void HidCalcNextReportTime( void );

/**
 *  \brief Capture the time of sending the current HID Report.
 *
 *  If the USB Host has previously sent a valid HID Set_Idle request with
 *    a duration of zero or greater than the default reporting interval,
 *    the device sends HID Reports periodically or when the value of the
 *    payload has changed.
 *
 *  This function captures the time when the HID Report was sent so that
 *    a subsequent call to HidCalNextReportTime() can calculate the time
 *    to send the next periodic HID Report.
 */
void HidCaptureReportTime( void );

XUD_Result_t HidInterfaceClassRequests(
  XUD_ep c_ep0_out,
  XUD_ep c_ep0_in,
  REFERENCE_PARAM( USB_SetupPacket_t, sp ));

/**
 *  \brief Indicate whether to send a HID Report based on elapsed time.
 *
 *  If the USB Host has previously sent a valid HID Set_Idle request with
 *    a duration of zero or greater than the default reporting interval,
 *    the device sends HID Reports periodically or when the value of the
 *    payload has changed.
 *
 *  This function monitors the passage of time and reports to the caller
 *    whether or not the time to send the next periodic HID Report has
 *    elapsed.
 *
 *  \return   A Boolean value indicating whether or not to send the HID Report.
 *  \retval   1 -- Do not send the HID Report
 *  \retval   0 -- Send the HID Report
 */
unsigned HidIsSetIdleSilenced( void );

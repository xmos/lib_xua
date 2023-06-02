// Copyright 2019-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) Class Request functions
 *
 * Document section numbers refer to the HID Device Class Definition, version 1.11.
 */

#ifndef __XUA_HID_H__
#define __XUA_HID_H__

#include <xs1.h>
#include <xccompat.h>
#include "xud.h"
#include "xud_std_requests.h"

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
 * Parameters:
 *
 *  @param[in]  id  The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                  A value of zero means the application does not use Report IDs.
 *
 *  \return   A Boolean value indicating whether or not to send the HID Report.
 *  \retval   1 -- Do not send the HID Report
 *  \retval   0 -- Send the HID Report
 */
unsigned HidIsSetIdleSilenced( const unsigned id );

#endif // __XUA_HID_H__

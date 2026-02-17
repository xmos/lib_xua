// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_USB_REQUESTS_H
#define DFU_USB_REQUESTS_H

#include <xccompat.h>

#include "xud_device.h"
#include "dfu_interface.h"

// TODO - refactor to remove "DFU_mode_active" param.

/* Handle USB reset events
 * 
 * CONTRACT: with lib_xua
 * Called from endpoint 0 init code to determine if the device should start direct into DFU mode.
 * Called from endpoint 0 handler when a USB reset event is detected.
 * 
 * Returns 1 if the device should be in DFU mode, 0 if it should be in application mode.
 */
int DFUReportResetState();

/* DFU API entry point for requests received over USB */
int DFUDeviceRequests(XUD_ep c_ep0_out, NULLABLE_REFERENCE_PARAM(XUD_ep, ep0_in), REFERENCE_PARAM(USB_SetupPacket_t, sp),
        unsigned int altInterface, CLIENT_INTERFACE(i_dfu, dfuInterface), REFERENCE_PARAM(int, reset));

/* Helper function for C */
void DFUDelay(unsigned d);

/* Handle XMOS specific DFU requests
 * 
 * Returns XUD_RES_OKAY if request was handled, XUD_RES_ERR if request was not recognised/handled.
 * 
 * CONTRACT: with lib_xua
 * Called from endpoint 0 handler when a vendor request is received.
 */
int dfu_usb_vendor_requests(XUD_ep ep0_out, XUD_ep ep0_in, REFERENCE_PARAM(USB_SetupPacket_t, sp), CLIENT_INTERFACE(i_dfu, dfuInterface), int DFU_mode_active);

/* Handle standard DFU requests
 *
 * Returns XUD_RES_OKAY if request was handled, XUD_RES_ERR if request was not recognised/handled.
 * 
 * CONTRACT: with lib_xua
 * Called from endpoint 0 handler when a class request to the DFU interface is received.
 */
int dfu_usb_class_int_requests(XUD_ep ep0_out, XUD_ep ep0_in, REFERENCE_PARAM(USB_SetupPacket_t, sp), CLIENT_INTERFACE(i_dfu, dfuInterface), NULLABLE_RESOURCE(chanend, c_aud_ctl), int DFU_mode_active);

// TODO - make parameter user customisable via macro or something, and remove from DFU interface since this is really a user callback and not a DFU interface function
void DFUNotifyEntryCallback(NULLABLE_RESOURCE(chanend, c_aud_ctl));

/* Reboot the device */
void device_reboot(void);

#endif /* DFU_USB_REQUESTS_H */

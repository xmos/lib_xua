// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_USB_REQUESTS_H
#define DFU_USB_REQUESTS_H

#include <xccompat.h>

#include "xud_device.h"
#include "dfu_interface.h"

// TODO - refactor to remove "DFU_mode_active" param.

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

// TODO - define weak function in C file.
// TODO - make parameter user customisable via macro or something, and remove from DFU interface since this is really a user callback and not a DFU interface function
void DFUNotifyEntryCallback(NULLABLE_RESOURCE(chanend, c_aud_ctl));

#endif /* DFU_USB_REQUESTS_H */

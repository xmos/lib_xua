// Copyright 2011-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XUA_DFU_API_H
#define XUA_DFU_API_H

#include <xccompat.h>
#include <stddef.h>

#include "xua.h"
#include "xud_device.h"
#include "dfu_types.h"

int DFUReportResetState(NULLABLE_RESOURCE(chanend , c_user_cmd));
int DFUDeviceRequests(XUD_ep c_ep0_out, NULLABLE_REFERENCE_PARAM(XUD_ep, ep0_in), REFERENCE_PARAM(USB_SetupPacket_t, sp),
        NULLABLE_RESOURCE(chanend, c_user_cmd), unsigned int altInterface, CLIENT_INTERFACE(i_dfu, dfuInterface), REFERENCE_PARAM(int, reset));

/* Helper function for C */
void DFUDelay(unsigned d);

#endif /* XUA_DFU_API_H */

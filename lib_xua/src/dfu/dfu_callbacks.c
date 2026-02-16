// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xccompat.h>

#include "dfu_usb_requests.h"

void DFUNotifyEntryCallback(NULLABLE_RESOURCE(chanend, c_aud_ctl)) __attribute__ ((weak));
void DFUNotifyEntryCallback(NULLABLE_RESOURCE(chanend, c_aud_ctl))
{
    /* Weak function, meant to be overridden by user if they want to be notified when a DFU request is received. This is useful for example to stop audio before rebooting to DFU mode. */
    (void)c_aud_ctl; // Suppress unused parameter warning
}

// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_CONF_H
#define DFU_CONF_H

#include "xua_conf_full.h"
#include "uac_hwresources.h"

#define CLKBLK_DFU_FLASHLIB CLKBLK_FLASHLIB

#define DFU_ENABLE XUA_DFU_EN

#define DFU_USB_EN XUA_USB_EN

#define DFU_BCD_DEVICE BCD_DEVICE

#define DFU_CONFIG_USB_INBAND_FUNCTIONS (DFU_USB_EN && (XUA_XUD_TILE_NUM == 0))

/** See lib_dfu for documentation of the following. */

#ifndef POLL_TIMEOUT_DNLOAD_ENTRY_MSEC
#define POLL_TIMEOUT_DNLOAD_ENTRY_MSEC 0
#endif

#ifndef POLL_TIMEOUT_DNLOAD_ERASE_MSEC
#define POLL_TIMEOUT_DNLOAD_ERASE_MSEC 8
#endif

#ifndef POLL_TIMEOUT_DNLOAD_FIRST_WRITE_MSEC
#define POLL_TIMEOUT_DNLOAD_FIRST_WRITE_MSEC 0
#endif

#ifndef POLL_TIMEOUT_DNLOAD_WRITE_MSEC
#define POLL_TIMEOUT_DNLOAD_WRITE_MSEC 0
#endif

#ifndef POLL_TIMEOUT_DNLOAD_MANIFEST_MSEC
#define POLL_TIMEOUT_DNLOAD_MANIFEST_MSEC 0
#endif

#endif /* DFU_CONF_H */

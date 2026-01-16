// Copyright 2025-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __DEVICE_ACCESS_USB_H__
#define __DEVICE_ACCESS_USB_H__

#ifdef __cplusplus
extern "C" {
#endif

int control_init_usb(int vendor_id, int product_id);
int control_cleanup_usb(void);
int control_read_command(uint8_t resid, uint8_t cmd,
                     uint8_t payload[], size_t payload_len);
int control_write_command(uint8_t resid, uint8_t cmd,
                      const uint8_t payload[], size_t payload_len);

#ifdef __cplusplus
}
#endif

#endif

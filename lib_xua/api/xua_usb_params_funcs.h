// Copyright 2017-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __XUA_API_H__
#define __XUA_API_H__

#include <stdint.h>

void set_usb_to_device_rate(uint32_t rate);
void set_device_to_usb_rate(uint32_t rate);
void set_usb_to_device_bit_res(uint32_t rate);
void set_device_to_usb_bit_res(uint32_t rate);

uint32_t get_usb_to_device_rate();
uint32_t get_device_to_usb_rate();
uint32_t get_usb_to_device_bit_res();
uint32_t get_device_to_usb_bit_res();

#endif  //__XUA_API_H__

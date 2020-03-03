// Copyright (c) 2017-2020, XMOS Ltd, All rights reserved

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

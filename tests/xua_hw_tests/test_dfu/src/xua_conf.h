// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_CONF_H_
#define _XUA_CONF_H_

#define XUD_TILE (0)
#define AUDIO_IO_TILE (1)

#define MCLK_441 (512 * 44100)
#define MCLK_48 (512 * 48000)

#define XUA_DFU_EN (1)

#define PRODUCT_STR "XUA DFU Test"
#define PID_AUDIO_2 (0x0016)
#define DFU_PID (0xD000 + PID_AUDIO_2)

#ifndef BCD_DEVICE
#error BCD_DEVICE must be defined in APP_COMPILER_FLAGS_<cfg>
#endif

#include "user_main.h"

#endif

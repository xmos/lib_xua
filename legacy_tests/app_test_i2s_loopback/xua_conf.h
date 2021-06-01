// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __custom_defines_h__
#define __custom_defines_h__

#define EXCLUDE_USB_AUDIO_MAIN
#define XUA_NUM_PDM_MICS 0
#define XUD_TILE 1
#define AUDIO_IO_TILE 0
#define MIXER 0
#define MCLK_441 (512 * 44100)
#define MCLK_48 (512 * 48000)
#define MIN_FREQ 44100
#define MAX_FREQ 192000
#define SPDIF_TX_INDEX 0
#define VENDOR_STR "XMOS"
#define VENDOR_ID 0x20B1
#define PRODUCT_STR_A2 "Test device"
#define PRODUCT_STR_A1 "Test device"
#define PID_AUDIO_1 1
#define PID_AUDIO_2 2
#define AUDIO_CLASS 2
#define AUDIO_CLASS_FALLBACK 0
#define BCD_DEVICE 0x1234
#define XUA_DFU_EN          0
#define MIC_DUAL_ENABLED 1        //Use single thread, dual PDM mic
#define XUA_MIC_FRAME_SIZE 240

#endif // __custom_defines_h__

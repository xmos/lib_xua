/* Required by lib_xua at the moment. To be replaced with lib_xua_conf.h"
 */

// Copyright (c) 2016, XMOS Ltd, All rights reserved
#ifndef __custom_defines_h__
#define __custom_defines_h__

#define NUM_USB_CHAN_OUT 2
#define NUM_USB_CHAN_IN 0
#define I2S_CHANS_DAC 2
#define I2S_CHANS_ADC 0
#define EXCLUDE_USB_AUDIO_MAIN
#define NUM_PDM_MICS 0
#define XUD_TILE 1
#define AUDIO_IO_TILE 0
#define MIXER 0
#define MCLK_441 (512 * 44100)
#define MCLK_48 (512 * 48000)
#define MIN_FREQ 48000
#define MAX_FREQ 48000
#define SPDIF_TX_INDEX 0
#define VENDOR_STR "XMOS"
#define VENDOR_ID 0x20B1
#define PRODUCT_STR_A2 "XUA Simple"
#define PRODUCT_STR_A1 "XUA Simple"
#define PID_AUDIO_1 1
#define PID_AUDIO_2 2
#define AUDIO_CLASS 2
#define AUDIO_CLASS_FALLBACK 0
#define BCD_DEVICE 0x1234
#define XUA_DFU_EN 0

/* TODO */
#define XUA_DFU XUA_DFU_EN

#endif // __custom_defines_h__


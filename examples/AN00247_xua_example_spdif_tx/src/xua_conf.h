// Copyright (c) 2017-2019, XMOS Ltd, All rights reserved

#ifndef _XUA_CONF_H_ 
#define _XUA_CONF_H_

#define NUM_USB_CHAN_OUT 2
#define NUM_USB_CHAN_IN 0
#define I2S_CHANS_DAC 0
#define I2S_CHANS_ADC 0
#define MCLK_441 (512 * 44100)
#define MCLK_48 (512 * 48000)
#define MIN_FREQ 48000
#define MAX_FREQ 48000

#define EXCLUDE_USB_AUDIO_MAIN

#define XUA_SPDIF_TX_EN 1
#define SPDIF_TX_INDEX 0
#define VENDOR_STR "XMOS"
#define VENDOR_ID 0x20B1
#define PRODUCT_STR_A2 "XUA SPDIF Example"
#define PRODUCT_STR_A1 "XUA SPDIF Example"
#define PID_AUDIO_1 1
#define PID_AUDIO_2 2
#define AUDIO_CLASS 2
#define AUDIO_CLASS_FALLBACK 0
#define BCD_DEVICE 0x1234
#define XUA_DFU_EN 0
#define MIC_DUAL_ENABLED 0          // Use multiple thread

#endif

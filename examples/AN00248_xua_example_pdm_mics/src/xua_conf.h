// Copyright 2017-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _XUA_CONF_H_
#define _XUA_CONF_H_

#define NUM_USB_CHAN_OUT    2
#define NUM_USB_CHAN_IN     2
#define I2S_CHANS_DAC       2
#define I2S_CHANS_ADC       0
#define MCLK_441            (512 * 44100)
#define MCLK_48             (512 * 48000)
#define DEFAULT_FREQ        32000
#define MIN_FREQ            DEFAULT_FREQ
#define MAX_FREQ            DEFAULT_FREQ

#define AUDIO_IO_TILE       1
#define XUD_TILE            0
#define MIXER               0
#define HID_CONTROLS        0

#define EXCLUDE_USB_AUDIO_MAIN

#define XUA_NUM_PDM_MICS    2
#define VENDOR_STR "XMOS"
#define VENDOR_ID 0x20B1
#define PRODUCT_STR_A2 "XUA PDM Mics Example"
#define PRODUCT_STR_A1 "XUA PDM Mics Example"
#define PID_AUDIO_1 1
#define PID_AUDIO_2 2
#define AUDIO_CLASS 2
#define AUDIO_CLASS_FALLBACK 0
#define BCD_DEVICE 0x1234
#define XUA_DFU_EN 0

#endif

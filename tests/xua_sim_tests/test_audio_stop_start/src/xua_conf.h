// Copyright 2016-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_CONF_H_
#define _XUA_CONF_H_

#define EXCLUDE_USB_AUDIO_MAIN
#define XUA_NUM_PDM_MICS 0
#define XUA_XUD_TILE_NUM 0
#define XUA_AUDIO_IO_TILE_NUM 1
#define MIXER 0

#ifndef MCLK_441
#define MCLK_441 (512 * 44100)
#endif

#ifndef MCLK_48
#define MCLK_48 (512 * 48000)
#endif

#ifndef NUM_USB_CHAN_IN
#define NUM_USB_CHAN_IN		2
#endif
#ifndef NUM_USB_CHAN_OUT
#define NUM_USB_CHAN_OUT	2
#endif
#define I2S_CHANS_ADC		NUM_USB_CHAN_IN
#define I2S_CHANS_DAC		NUM_USB_CHAN_OUT

#define MIN_FREQ (44100)
#define MAX_FREQ (192000)
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
#define XUA_DFU_EN          1

#endif

// Copyright 2017-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _XUA_CONF_H_
#define _XUA_CONF_H_

#define NUM_USB_CHAN_OUT      2     /* Number of channels from host to device */
#define NUM_USB_CHAN_IN       0     /* Number of channels from device to host */
#define I2S_CHANS_DAC         2     /* Number of I2S channels out of xCORE */
#define I2S_CHANS_ADC         0     /* Number of I2S channels in to xCORE */
#define MCLK_441  (512 * 44100)     /* 44.1kHz family master clock frequency */
#define MCLK_48   (512 * 48000)     /* 48kHz family master clock frequency */
#define MIN_FREQ  48000             /* Minimum sample rate */
#define MAX_FREQ  48000             /* Maximum sample rate */

#define EXCLUDE_USB_AUDIO_MAIN

#define VENDOR_STR      "XMOS"
#define VENDOR_ID       0x20B1
#define PRODUCT_STR_A2  "XUA Example"
#define PRODUCT_STR_A1  "XUA Example"
#define PID_AUDIO_1     1
#define PID_AUDIO_2     2
#define XUA_DFU_EN      0           /* Disable DFU (for simplicity of example */
#define MIC_DUAL_ENABLED 0          // Use multi-threaded design

#endif

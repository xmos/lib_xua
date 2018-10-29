// Copyright (c) 2017-2018, XMOS Ltd, All rights reserved

#ifndef _XUA_CONF_H_ 
#define _XUA_CONF_H_

#define NUM_USB_CHAN_OUT      2     /* Number of channels from host to device */
#define NUM_USB_CHAN_IN       2     /* Number of channels from device to host */
#define I2S_CHANS_DAC         2     /* Number of I2S channels out of xCORE */
#define I2S_CHANS_ADC         2     /* Number of I2S channels in to xCORE */
#define MCLK_441  (512 * 44100)     /* 44.1kHz family master clock frequency */
#define MCLK_48   (512 * 48000)     /* 48kHz family master clock frequency */
#define MIN_FREQ  48000             /* Minimum sample rate */
#define MAX_FREQ  48000             /* Maximum sample rate */

#define EXCLUDE_USB_AUDIO_MAIN

#define VENDOR_STR            "XMOS"
#define VENDOR_ID             0x20B1
#define PRODUCT_STR_A2        "XUA Lite Class 2"
#define PRODUCT_STR_A1        "XUA Lite Class 1"
#define PID_AUDIO_1           1   
#define PID_AUDIO_2           2
#define XUA_DFU_EN            0           /* Disable DFU (for simplicity of example) */

#define UAC_FORCE_FEEDBACK_EP 1
#define XUA_LITE              1           // Use simple/optimised USB buffer tasks
#define AUDIO_CLASS           2

#endif
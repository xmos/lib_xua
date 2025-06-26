// Copyright 2017-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _XUA_CONF_H_
#define _XUA_CONF_H_

#define NUM_USB_CHAN_OUT      4     /* Number of channels from host to device */
#define NUM_USB_CHAN_IN       4     /* Number of channels from device to host */
#define MCLK_441  (512 * 44100)     /* 44.1kHz family master clock frequency */
#define MCLK_48   (512 * 48000)     /* 48kHz family master clock frequency */
#define MIN_FREQ  44100             /* Minimum sample rate */
#define MAX_FREQ  192000            /* Maximum sample rate */

#define EXCLUDE_USB_AUDIO_MAIN		/* Use our own main function */
#define XUA_WRAPPER		1 			/* Just use the USB host side tasks, use own audio function */

#define VENDOR_STR      "XMOS"
#define VENDOR_ID       0x20B1
#define PRODUCT_STR_A2  "XUA Wrapper Example"
#define PRODUCT_STR_A1  "XUA Wrapper Example"
#define PID_AUDIO_1     1
#define PID_AUDIO_2     2
#define XUA_DFU_EN      0           /* Disable DFU (for simplicity of example */

#endif

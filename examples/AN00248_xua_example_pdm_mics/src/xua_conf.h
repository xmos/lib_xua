// Copyright 2017-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _XUA_CONF_H_
#define _XUA_CONF_H_

#include <platform.h>

#define NUM_USB_CHAN_OUT                    0                /* Number of channels from host to device */
#define NUM_USB_CHAN_IN                     2                /* Number of channels from device to host */
#define I2S_CHANS_DAC                       2                /* Number of I2S channels out of xCORE */
#define I2S_CHANS_ADC                       0                /* Number of I2S channels in to xCORE */
#define MCLK_441                            (512 * 44100)    /* 44.1kHz family master clock frequency */
#define MCLK_48                             (512 * 48000)    /* 48kHz family master clock frequency */
#define XUA_PDM_MIC_FREQ                    48000            /* Currently sample rate changes are not supported for PDM mics */
#define MIN_FREQ                            XUA_PDM_MIC_FREQ /* Minimum sample rate */
#define MAX_FREQ                            XUA_PDM_MIC_FREQ /* Maximum sample rate */

#define AUDIO_IO_TILE 						1
#define PORT_MCLK_IN_USB 					XS1_PORT_1D
#define PORT_MCLK_COUNT 					XS1_PORT_16B
#define PORT_I2S_DAC0						PORT_I2S_DAC_DATA

#define XUA_NUM_PDM_MICS                    2
#define MIC_ARRAY_CONFIG_PORT_MCLK          PORT_MCLK_IN
#define MIC_ARRAY_CONFIG_PORT_PDM_CLK       PORT_PDM_CLK
#define MIC_ARRAY_CONFIG_PORT_PDM_DATA      PORT_PDM_DATA
#define MIC_ARRAY_CONFIG_CLOCK_BLOCK_A      XS1_CLKBLK_1      
#define MIC_ARRAY_CONFIG_CLOCK_BLOCK_B      XS1_CLKBLK_2    /* Second clock needed as we use DDR */
#define PDM_TILE 							1

#define AUDIO_CLASS                         1
#define VENDOR_STR                          "XMOS"
#define VENDOR_ID                           0x20B1
#define PRODUCT_STR_A2                      "XUA PDM Example"
#define PRODUCT_STR_A1                      "XUA PDM Example"
#define PID_AUDIO_1                         1
#define PID_AUDIO_2                         2
#define XUA_DFU_EN                          0               /* Disable DFU for simplicity of example */

#include "user_main.h"

#endif

// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _XUA_CONF_H_
#define _XUA_CONF_H_

/*** Toolchain imports ***/
#include <xs1.h>
#include <platform.h>

/*** IO config ***/
#define NUM_USB_CHAN_OUT 0 /* Number of channels from host to device */
#define NUM_USB_CHAN_IN  2 /* Number of channels from device to host */
#define I2S_CHANS_DAC    0 /* Number of I2S channels out of xCORE */
#define I2S_CHANS_ADC    0 /* Number of I2S channels in to xCORE */
#define AUDIO_IO_TILE    1 /* PDM mics are on tile[1] on the VDK */
#define XUD_TILE         0 /* Cannot use USB and mics on tile[1] on the VDK */

/*** Clocking config ***/
/* This example supports two modes of operation: one where the USB clock is
 * driven by the tile[0] ref clock, and one where it is driven by the app PLL
 * via a link cable. Set USE_APP_PLL_TILE_0 to 1 if this link is in use. */
#ifndef USE_APP_PLL_TILE_0
#define USE_APP_PLL_TILE_0   0
#endif

#if USE_APP_PLL_TILE_0
/* In this mode, we have a hardwired link between the application PLL on
 * tile[1] and a pin on tile[0]. Therefore, set up XUA to use sw_pll and
 * specify some ports for it to use. */
#define XUA_USE_SW_PLL       1
#define USB_MCLK_PORT        XS1_PORT_1D  /* tile[0] port wired to app PLL */
#define USB_MCLK_COUNT_PORT  XS1_PORT_16B /* Extra port to count MCLK ticks */
#define USB_MCLK_CLOCK_BLOCK XS1_CLKBLK_1 /* Clock block to use for USB */
#else
/* In this mode, we do not have a hardwired link. Use the internal reference
 * clock instead. */
#define XUA_USE_SW_PLL       0
#define FB_USE_REF_CLOCK     1
#endif

#define MCLK_441             (512 * 44100) /* 44.1 kHz family MCLK frequency */
#define MCLK_48              (512 * 48000) /* 48 kHz family MCLK frequency */

/*** Mic array config ***/
/* Dynamic sample rate changes are currently unsupported in lib_mic_array.
 * Therefore, set both MIN_FREQ and MAX_FREQ to the same value, 48 kHz. */
#define XUA_PDM_MIC_FREQ               48000           
#define MIN_FREQ                       XUA_PDM_MIC_FREQ
#define MAX_FREQ                       XUA_PDM_MIC_FREQ

/* These ports are defined in the .xn file. */
#define XUA_NUM_PDM_MICS               2
#define MIC_ARRAY_CONFIG_PORT_MCLK     PORT_MCLK_IN
#define MIC_ARRAY_CONFIG_PORT_PDM_CLK  PORT_MIC_CLK
#define MIC_ARRAY_CONFIG_PORT_PDM_DATA PORT_MIC_DATA
#define MIC_ARRAY_CONFIG_CLOCK_BLOCK_A XS1_CLKBLK_1
#define MIC_ARRAY_CONFIG_CLOCK_BLOCK_B XS1_CLKBLK_2 /* Required as using DDR */

/*** XUA config ***/
#define EXCLUDE_USB_AUDIO_MAIN

#define AUDIO_CLASS            2
#define VENDOR_STR             "XMOS"
#define VENDOR_ID              0x20B1
#define PRODUCT_STR_A2         "XUA PDM Example UAC2"
#define PRODUCT_STR_A1         "XUA PDM Example UAC1"
#define PID_AUDIO_1            1
#define PID_AUDIO_2            2
#define XUA_DFU_EN             0 /* Disable DFU for simplicity of example */

#endif // ifndef _XUA_CONF_H_

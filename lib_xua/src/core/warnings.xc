// Copyright 2013-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/*
 * Warnings relating to configuration defines located in this XC source file rather than the xua_conf.h
 * header file in order to avoid multiple warnings being issued when the xua_conf.h header file is
 * included in multiple files.
 */

#include "xua_conf_full.h"

#if XUA_USB_EN

#ifndef DEFAULT_FREQ
#warning DEFAULT_FREQ not defined. Using MIN_FREQ
#endif

#ifndef MIN_FREQ
#warning MIN_FREQ not defined. Using 44100
#endif

#ifndef MAX_FREQ
#warning MAX_FREQ not defined. Using 192000
#endif

#ifdef SPDIF_TX
#ifndef SPDIF_TX_INDEX
#warning SPDIF_TX_INDEX not defined! Using 0
#endif
#endif

#ifndef VENDOR_STR
#warning VENDOR_STR not defined. Using "XMOS"
#endif

#ifndef VENDOR_ID
#warning VENDOR_ID not defined. Using XMOS vendor ID (0x20B1)
#endif

#ifndef PRODUCT_STR_A2
#warning PRODUCT_STR_A2 not defined. Using default XMOS string
#endif

#ifndef PRODUCT_STR_A1
#warning PRODUCT_STR_A1 not defined. Using default XMOS string
#endif

#ifndef BCD_DEVICE
#warning BCD_DEVICE not defined. Using XMOS release version number
#endif

#if (XUA_AUDIO_CLASS_FS == 1)
#ifndef PID_AUDIO_1
#warning PID_AUDIO_1 not defined. Using 0x0003
#endif
#endif

#ifndef PID_AUDIO_2
#warning PID_AUDIO_2 not defined. Using 0x0002
#endif

#ifndef AUDIO_CLASS
#warning AUDIO_CLASS not defined, using 2
#endif

/* Sanity check on FS channel counts */
#if (NUM_USB_CHAN_OUT_FS > NUM_USB_CHAN_OUT)
#error NUM_USB_CHAN_OUT_FS expected to be less than or equal to NUM_USB_CHAN_OUT
#endif

#if (NUM_USB_CHAN_IN_FS > NUM_USB_CHAN_IN)
#error NUM_USB_CHAN_IN_FS expected to be less than or equal to NUM_USB_CHAN_IN
#endif

/* Run some checks WRT to low power modes */
#if XUA_LOW_POWER_NON_STREAMING
#if MIXER
#warning Enabling MIXER when XUA_LOW_POWER_NON_STREAMING is enabled will result in the mixer stopping when USB audio streams are not active. Is this what you wanted?
#endif
#if (NUM_USB_CHAN_OUT == 0 && NUM_USB_CHAN_IN == 0)
#error Disable XUA_LOW_POWER_NON_STREAMING if you wish to have a system with no USB audio streams. These features are incompatible.
#endif
#endif

#ifdef XUA_CHAN_BUFF_CTRL
#warning Using channel to control buffering - this may reduce performance but improve power consumption
#endif

#ifdef AUDIO_IO_TILE
#warning "Use of AUDIO_IO_TILE is to be deprecated, use XUA_AUDIO_IO_TILE_NUM instead"
#endif

#ifdef XUD_TILE
#warning "Use of XUD_TILE is to be deprecated, use XUA_XUD_TILE_NUM instead"
#endif

#ifdef MIDI_TILE
#warning "Use of MIDI_TILE is to be deprecated, use XUA_MIDI_TILE_NUM instead"
#endif

#ifdef SPDIF_TX_TILE
#warning "Use of SPDIF_TX_TILE is to be deprecated, use XUA_SPDIF_TX_TILE_NUM instead"
#endif

#ifdef PDM_TILE
#warning "Use of PDM_TILE is to be deprecated, use XUA_MIC_PDM_TILE_NUM instead"
#endif

#ifdef PLL_REF_TILE
#warning "Use of PLL_REF_TILE is to be deprecated, use XUA_PLL_REF_TILE_NUM instead"
#endif

#endif

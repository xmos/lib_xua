// Copyright 2013-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/*
Warnings relating to configuration defines located in this XC source file rather than the xua_conf.h header file in order to avoid multiple warnings being issued when the xua_conf.h header file is included in multiple files.
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

#if (AUDIO_CLASS == 1) || (AUDIO_CLASS_FALLBACK)
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

#ifndef AUDIO_CLASS_FALLBACK
#warning AUDIO_CLASS_FALLBACK not defined, using 0 (i.e. disabled)
#endif

/* Sanity check on FS channel counts */
#if (NUM_USB_CHAN_OUT_FS > NUM_USB_CHAN_OUT)
#error NUM_USB_CHAN_OUT_FS expected to be less than or equal to NUM_USB_CHAN_OUT
#endif

#if (NUM_USB_CHAN_IN_FS > NUM_USB_CHAN_IN)
#error NUM_USB_CHAN_IN_FS expected to be less than or equal to NUM_USB_CHAN_IN
#endif

#endif

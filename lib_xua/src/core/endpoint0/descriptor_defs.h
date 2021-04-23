// Copyright 2015-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __DESCRIPTOR_DEFS_H__
#define __DESCRIPTOR_DEFS_H__

/*
    Include xua.h to pick up the #defines of NUM_USB_CHAN_IN and NUM_USB_CHAN_OUT.
 */
#include "xua.h"

#if (NUM_USB_CHAN_IN > 0) && (NUM_USB_CHAN_OUT > 0)
#define AUDIO_INTERFACE_COUNT 3
#elif (NUM_USB_CHAN_IN > 0) || (NUM_USB_CHAN_OUT > 0)
#define AUDIO_INTERFACE_COUNT 2
#else
#define AUDIO_INTERFACE_COUNT 1
#endif

/* Endpoint address defines */
#define ENDPOINT_ADDRESS_IN_CONTROL               (ENDPOINT_NUMBER_IN_CONTROL | 0x80)
#define ENDPOINT_ADDRESS_IN_FEEDBACK              (ENDPOINT_NUMBER_IN_FEEDBACK | 0x80)
#define ENDPOINT_ADDRESS_IN_AUDIO                 (ENDPOINT_NUMBER_IN_AUDIO | 0x80)
#define ENDPOINT_ADDRESS_IN_INTERRUPT             (ENDPOINT_NUMBER_IN_INTERRUPT | 0x80)
#define ENDPOINT_ADDRESS_IN_MIDI                  (ENDPOINT_NUMBER_IN_MIDI | 0x80)
#define ENDPOINT_ADDRESS_IN_HID                   (ENDPOINT_NUMBER_IN_HID | 0x80)
#define ENDPOINT_ADDRESS_IN_IAP_INT               (ENDPOINT_NUMBER_IN_IAP_INT | 0x80)
#define ENDPOINT_ADDRESS_IN_IAP                   (ENDPOINT_NUMBER_IN_IAP | 0x80)
#define ENDPOINT_ADDRESS_IN_IAP_EA_NATIVE_TRANS   (ENDPOINT_NUMBER_IN_IAP_EA_NATIVE_TRANS | 0x80)

#define ENDPOINT_ADDRESS_OUT_CONTROL              (ENDPOINT_NUMBER_OUT_CONTROL)
#define ENDPOINT_ADDRESS_OUT_AUDIO                (ENDPOINT_NUMBER_OUT_AUDIO)
#define ENDPOINT_ADDRESS_OUT_MIDI                 (ENDPOINT_NUMBER_OUT_MIDI)
#define ENDPOINT_ADDRESS_OUT_IAP                  (ENDPOINT_NUMBER_OUT_IAP)
#define ENDPOINT_ADDRESS_OUT_IAP_EA_NATIVE_TRANS  (ENDPOINT_NUMBER_OUT_IAP_EA_NATIVE_TRANS)

/* Interface numbers enum */
enum USBInterfaceNumber
{
    INTERFACE_NUMBER_AUDIO_CONTROL = 0,
#if (NUM_USB_CHAN_OUT > 0)
    INTERFACE_NUMBER_AUDIO_OUTPUT,
#endif
#if (NUM_USB_CHAN_IN > 0)
    INTERFACE_NUMBER_AUDIO_INPUT,
#endif
#if defined(MIDI) && (MIDI != 0)
    INTERFACE_NUMBER_MIDI_CONTROL,
    INTERFACE_NUMBER_MIDI_STREAM,
#endif
#if defined(USB_CONTROL_DESCS) && (USB_CONTROL_DESCS != 0)
    INTERFACE_NUMBER_MISC_CONTROL,
#endif
#if defined(XUA_DFU_EN) && (XUA_DFU_EN != 0)
    INTERFACE_NUMBER_DFU,
#endif
#if defined(IAP) && (IAP != 0)
    INTERFACE_NUMBER_IAP,
#if defined(IAP_EA_NATIVE_TRANS) && (IAP_EA_NATIVE_TRANS != 0)
    INTERFACE_NUMBER_IAP_EA_NATIVE_TRANS,
#endif
#endif
#if( 0 < HID_CONTROLS )
    INTERFACE_NUMBER_HID,
#endif
    INTERFACE_COUNT          /* End marker */
};

#if( 0 < HID_CONTROLS )
#define ENDPOINT_INT_INTERVAL_IN_HID 0x08
#endif

#endif

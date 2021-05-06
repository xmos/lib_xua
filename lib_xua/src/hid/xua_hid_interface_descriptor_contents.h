// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) Interface descriptor
 *
 * This file lists the contents of the HID Interface descriptor returned during enumeration.
 */

#ifndef _HID_INTERFACE_DESCRIPTOR_CONTENTS_
#define _HID_INTERFACE_DESCRIPTOR_CONTENTS_

#include "descriptor_defs.h"

#if (AUDIO_CLASS == 1)

    /* HID interface descriptor */
    0x09,                                 /* 0  bLength : Size of descriptor in Bytes */
    0x04,                                 /* 1  bDescriptorType (Interface: 0x04)*/
    INTERFACE_NUMBER_HID,                 /* 2  bInterfaceNumber : Number of interface */
    0x00,                                 /* 3  bAlternateSetting : Value used  alternate interfaces using SetInterface Request */
    0x01,                                 /* 4: bNumEndpoints : Number of endpoitns for this interface (excluding 0) */
    0x03,                                 /* 5: bInterfaceClass */
    0x00,                                 /* 6: bInterfaceSubClass - no boot device */
    0x00,                                 /* 7: bInterfaceProtocol*/
    0x00,                                 /* 8  iInterface */

#elif (AUDIO_CLASS == 2)

    .HID_Interface =
    {
        /* HID interface descriptor */
        .bLength            = 0x09,
        .bDescriptorType    = 0x04,
        .bInterfaceNumber   = INTERFACE_NUMBER_HID,
        .bAlternateSetting  = 0x00,                 /* alternate interfaces using SetInterface Request */
        .bNumEndpoints      = 0x01,
        .bInterfaceClass    = 0x03,
        .bInterfaceSubClass = 0x00,                 /* no boot device */
        .bInterfaceProtocol = 0x00,
        .iInterface         = 0x00,
    },

#else
    #error "Unknown Audio Class"
#endif

#endif // _HID_INTERFACE_DESCRIPTOR_CONTENTS_

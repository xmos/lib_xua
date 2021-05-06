// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) Endpoint descriptors
 *
 * This file lists the contents of the HID Endpoint descriptor returned during enumeration.
 */

#ifndef _HID_ENDPOINT_DESCRIPTOR_CONTENTS_
#define _HID_ENDPOINT_DESCRIPTOR_CONTENTS_

#include "descriptor_defs.h"

#if (AUDIO_CLASS == 1)

    /* HID Endpoint descriptor (IN) */
    0x07,                                 /* 0  bLength */
    0x05,                                 /* 1  bDescriptorType */
    ENDPOINT_ADDRESS_IN_HID,              /* 2  bEndpointAddress  */
    0x03,                                 /* 3  bmAttributes (INTERRUPT) */
    0x40,                                 /* 4  wMaxPacketSize */
    0x00,                                 /* 5  wMaxPacketSize */
    ENDPOINT_INT_INTERVAL_IN_HID,         /* 6  bInterval */

#elif (AUDIO_CLASS == 2)

    .HID_In_Endpoint =
    {
        /* Endpoint descriptor (IN) */
        .bLength            = 0x7,
        .bDescriptorType    = 5,
        .bEndpointAddress   = ENDPOINT_ADDRESS_IN_HID,
        .bmAttributes       = 3,
        .wMaxPacketSize     = 64,
        .bInterval          = ENDPOINT_INT_INTERVAL_IN_HID,
    },

#else
    #error "Unknown Audio Class"
#endif

#endif // _HID_ENDPOINT_DESCRIPTOR_CONTENTS_

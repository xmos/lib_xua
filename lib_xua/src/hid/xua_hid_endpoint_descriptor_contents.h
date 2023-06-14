// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) Endpoint descriptors
 *
 * This file lists the contents of the HID Endpoint descriptor returned during enumeration.
 */

#ifndef _HID_ENDPOINT_DESCRIPTOR_CONTENTS_
#define _HID_ENDPOINT_DESCRIPTOR_CONTENTS_

#include "descriptor_defs.h"

#define HID_ENDPOINT_DESCRIPTOR_LENGTH          ( 0x07 )    /* Size of descriptor in Bytes */
#define HID_ENDPOINT_DESCRIPTOR_TYPE            ( 0x05 )    /* Endpoint 0x05 */
#define HID_ENDPOINT_ATTRIBUTES                 ( 0x03 )    /* Interrupt */
#define HID_ENDPOINT_DESCRIPTOR_PACKET_SIZE_LO  ( 0x40 )
#define HID_ENDPOINT_DESCRIPTOR_PACKET_SIZE_HI  ( 0x00 )

#endif // _HID_ENDPOINT_DESCRIPTOR_CONTENTS_

#if (AUDIO_CLASS == 1)

    /* HID Endpoint descriptor (IN) */
    HID_ENDPOINT_DESCRIPTOR_LENGTH,         /* 0  bLength */
    HID_ENDPOINT_DESCRIPTOR_TYPE,           /* 1  bDescriptorType */
    ENDPOINT_ADDRESS_IN_HID,                /* 2  bEndpointAddress  */
    HID_ENDPOINT_ATTRIBUTES,                /* 3  bmAttributes (INTERRUPT) */
    HID_ENDPOINT_DESCRIPTOR_PACKET_SIZE_LO, /* 4  wMaxPacketSize */
    HID_ENDPOINT_DESCRIPTOR_PACKET_SIZE_HI, /* 5  wMaxPacketSize */
    ENDPOINT_INT_INTERVAL_IN_HID,           /* 6  bInterval */

#if (HID_OUT_REQUIRED)

    /* HID Endpoint descriptor (OUT) */
    HID_ENDPOINT_DESCRIPTOR_LENGTH,         /* 0  bLength */
    HID_ENDPOINT_DESCRIPTOR_TYPE,           /* 1  bDescriptorType */
    ENDPOINT_ADDRESS_OUT_HID,               /* 2  bEndpointAddress  */
    HID_ENDPOINT_ATTRIBUTES,                /* 3  bmAttributes (INTERRUPT) */
    HID_ENDPOINT_DESCRIPTOR_PACKET_SIZE_LO, /* 4  wMaxPacketSize */
    HID_ENDPOINT_DESCRIPTOR_PACKET_SIZE_HI, /* 5  wMaxPacketSize */
    ENDPOINT_INT_INTERVAL_OUT_HID,          /* 6  bInterval */

#endif

#elif (AUDIO_CLASS == 2)

    .HID_In_Endpoint =
    {
        /* Endpoint descriptor (IN) */
        .bLength            = sizeof(USB_Descriptor_Endpoint_t),
        .bDescriptorType    = HID_ENDPOINT_DESCRIPTOR_TYPE,
        .bEndpointAddress   = ENDPOINT_ADDRESS_IN_HID,
        .bmAttributes       = HID_ENDPOINT_ATTRIBUTES,
        .wMaxPacketSize     = HID_ENDPOINT_DESCRIPTOR_PACKET_SIZE_LO,
        .bInterval          = ENDPOINT_INT_INTERVAL_IN_HID,
    },

#if (HID_OUT_REQUIRED)

    .HID_Out_Endpoint =
    {
        /* Endpoint descriptor (OUT) */
        .bLength            = sizeof(USB_Descriptor_Endpoint_t),
        .bDescriptorType    = HID_ENDPOINT_DESCRIPTOR_TYPE,
        .bEndpointAddress   = ENDPOINT_ADDRESS_OUT_HID,
        .bmAttributes       = HID_ENDPOINT_ATTRIBUTES,
        .wMaxPacketSize     = HID_ENDPOINT_DESCRIPTOR_PACKET_SIZE_LO,
        .bInterval          = ENDPOINT_INT_INTERVAL_OUT_HID,
    },

#endif

#else
    #error "Unknown Audio Class"
#endif

// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) Endpoint descriptors
 *
 * This file lists the contents of the HID Endpoint descriptor returned during enumeration.
 */

#ifndef _HID_DESCRIPTOR_CONTENTS_
#define _HID_DESCRIPTOR_CONTENTS_

#if (AUDIO_CLASS == 1)

    /* HID descriptor */
    0x09,                                 /* 0  bLength : Size of descriptor in Bytes */
    0x21,                                 /* 1  bDescriptorType (HID) */
    0x10,                                 /* 2  bcdHID */
    0x01,                                 /* 3  bcdHID */
    0x00,                                 /* 4  bCountryCode */
    0x01,                                 /* 5  bNumDescriptors */
    0x22,                                 /* 6  bDescriptorType[0] (Report) */
    sizeof(hidReportDescriptor) & 0xff,   /* 7  wDescriptorLength[0] */
    sizeof(hidReportDescriptor) >> 8,     /* 8  wDescriptorLength[0] */

#elif (AUDIO_CLASS == 2)

    .HID_Descriptor =
    {
        /* HID descriptor */
        .bLength            = 0x09,
        .bDescriptorType0   = 0x21,
        .bcdHID             =
        {
                              0x10,
                              0x01,
        },
        .bCountryCode       = 0x00,
        .bNumDescriptors    = 0x01,
        .bDescriptorType1   = 0x22,
        .wDescriptorLength1 =
        {
            sizeof(hidReportDescriptor) & 0xff,
            sizeof(hidReportDescriptor) >> 8,
        },
    },

#else
    #error "Unknown Audio Class"
#endif
    
#endif // _HID_DESCRIPTOR_CONTENTS_

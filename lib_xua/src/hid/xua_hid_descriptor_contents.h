// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) Endpoint descriptors
 *
 * This file lists the contents of the HID Endpoint descriptor returned during enumeration.
 */

#ifndef _HID_DESCRIPTOR_CONTENTS_
#define _HID_DESCRIPTOR_CONTENTS_

#include "xua_hid_descriptor.h"

#define HID_DESCRIPTOR_LENGTH_0    ( 0x09 ) /* Size of descriptor in Bytes */
#define HID_DESCRIPTOR_TYPE_0      ( 0x21 ) /* HID 0x21 */
#define HID_BCD_VERSION_LO         ( 0x10 ) /* HID class specification release */
#define HID_BCD_VERSION_HI         ( 0x01 )
#define HID_COUNTRY_CODE           ( 0x00 ) /* Country code of localized hardware */
#define HID_NUM_DESCRIPTORS        ( 0x01 ) /* Number of class descriptors */
#define HID_DESCRIPTOR_TYPE_1      ( 0x22 ) /* Type of 1st class descriptor, Report 0x22 */
#define HID_DESCRIPTOR_LENGTH_1_LO ( 0x00 ) /* Length of 1st class descriptor, set to zero */
#define HID_DESCRIPTOR_LENGTH_1_HI ( 0x00 ) /* since only pre-processor directives allowed here */

#endif // _HID_DESCRIPTOR_CONTENTS_

#if (AUDIO_CLASS == 1)

    /* HID descriptor */
    HID_DESCRIPTOR_LENGTH_0,     /* 0  bLength */
    HID_DESCRIPTOR_TYPE_0,       /* 1  bDescriptorType (HID) */
    HID_BCD_VERSION_LO,          /* 2  bcdHID */
    HID_BCD_VERSION_HI,          /* 3  bcdHID */
    HID_COUNTRY_CODE,            /* 4  bCountryCode */
    HID_NUM_DESCRIPTORS,         /* 5  bNumDescriptors */
    HID_DESCRIPTOR_TYPE_1,       /* 6  bDescriptorType[0] */
    HID_DESCRIPTOR_LENGTH_1_LO,  /* 7  wDescriptorLength[0] */
    HID_DESCRIPTOR_LENGTH_1_HI,  /* 8  wDescriptorLength[0] */

#elif (AUDIO_CLASS == 2)

    .HID_Descriptor =
    {
        /* HID descriptor */
        .bLength            = sizeof(USB_HID_Descriptor_t),
        .bDescriptorType0   = HID_DESCRIPTOR_TYPE_0,
        .bcdHID             =
        {
                              HID_BCD_VERSION_LO,
                              HID_BCD_VERSION_HI,
        },
        .bCountryCode       = HID_COUNTRY_CODE,
        .bNumDescriptors    = HID_NUM_DESCRIPTORS,
        .bDescriptorType1   = HID_DESCRIPTOR_TYPE_1,
        .wDescriptorLength1 =
        {
                              HID_DESCRIPTOR_LENGTH_1_LO,
                              HID_DESCRIPTOR_LENGTH_1_HI,
        },
    },

#else
    #error "Unknown Audio Class"
#endif

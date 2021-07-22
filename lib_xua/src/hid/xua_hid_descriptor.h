// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) descriptor
 *
 * This file defines the structure of the HID descriptor.
 * Document section numbers refer to the HID Device Class Definition, version 1.11.
 */

#ifndef _HID_DESCRIPTOR_
#define _HID_DESCRIPTOR_

#define HID_DESCRIPTOR_LENGTH_FIELD_OFFSET ( 7 )

/* USB HID Descriptor (section 6.2.1) */
typedef struct
{
    unsigned char bLength;               /* Size of the descriptor (bytes) */
    unsigned char bDescriptorType0;      /* Descriptor type, a constant, 0x21 (section 7.1) */
    unsigned char bcdHID[2];             /* HID class specification release */
    unsigned char bCountryCode;          /* Country code of localized hardware */
    unsigned char bNumDescriptors;       /* Number of class descriptors */
    unsigned char bDescriptorType1;      /* Type of 1st class descriptor (section 7.1) */
    unsigned char wDescriptorLength1[2]; /* Length in bytes of the 1st class descriptor */
} __attribute__((packed)) USB_HID_Descriptor_t;

#endif // _HID_DESCRIPTOR_

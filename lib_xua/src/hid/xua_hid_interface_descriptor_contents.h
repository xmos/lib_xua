// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) Interface descriptor
 *
 * This file lists the contents of the HID Interface descriptor returned during enumeration.
 */

#ifndef _HID_INTERFACE_DESCRIPTOR_CONTENTS_
#define _HID_INTERFACE_DESCRIPTOR_CONTENTS_

#include "descriptor_defs.h"

#define HID_INTERFACE_DESCRIPTOR_LENGTH         ( 0x09 )    /* Size of descriptor in Bytes */
#define HID_INTERFACE_DESCRIPTOR_TYPE           ( 0x04 )    /* Interface 0x04 */
#define HID_INTERFACE_ALTERNATE_SETTING         ( 0x00 )    /* Value used alternate interfaces using SetInterface Request */
#define HID_INTERFACE_NUMBER_OF_ENDPOINTS       ( 0x01 + HID_OUT_REQUIRED )
                                                            /* Number of endpoints for this interface (excluding 0) */
#define HID_INTERFACE_CLASS                     ( 0x03 )
#define HID_INTERFACE_SUBCLASS                  ( 0x00 )    /* No boot device */
#define HID_INTERFACE_PROTOCOL                  ( 0x00 )
#define HID_INTERFACE_STRING_DESCRIPTOR_INDEX   ( 0x00 )

#endif // _HID_INTERFACE_DESCRIPTOR_CONTENTS_

#if (AUDIO_CLASS == 1)

    /* HID interface descriptor */
    HID_INTERFACE_DESCRIPTOR_LENGTH,        /* 0  bLength */
    HID_INTERFACE_DESCRIPTOR_TYPE,          /* 1  bDescriptorType */
    INTERFACE_NUMBER_HID,                   /* 2  bInterfaceNumber : Number of interface */
    HID_INTERFACE_ALTERNATE_SETTING,        /* 3  bAlternateSetting */
    HID_INTERFACE_NUMBER_OF_ENDPOINTS,      /* 4: bNumEndpoints */
    HID_INTERFACE_CLASS,                    /* 5: bInterfaceClass */
    HID_INTERFACE_SUBCLASS,                 /* 6: bInterfaceSubClass */
    HID_INTERFACE_PROTOCOL,                 /* 7: bInterfaceProtocol*/
    HID_INTERFACE_STRING_DESCRIPTOR_INDEX,  /* 8  iInterface */

#elif (AUDIO_CLASS == 2)

    .HID_Interface =
    {
        /* HID interface descriptor */
        .bLength            = sizeof(USB_Descriptor_Interface_t),
        .bDescriptorType    = HID_INTERFACE_DESCRIPTOR_TYPE,
        .bInterfaceNumber   = INTERFACE_NUMBER_HID,
        .bAlternateSetting  = HID_INTERFACE_ALTERNATE_SETTING,
        .bNumEndpoints      = HID_INTERFACE_NUMBER_OF_ENDPOINTS,
        .bInterfaceClass    = HID_INTERFACE_CLASS,
        .bInterfaceSubClass = HID_INTERFACE_SUBCLASS,
        .bInterfaceProtocol = HID_INTERFACE_PROTOCOL,
        .iInterface         = HID_INTERFACE_STRING_DESCRIPTOR_INDEX,
    },

#else
    #error "Unknown Audio Class"
#endif

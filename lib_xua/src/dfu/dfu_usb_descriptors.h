// Copyright 2011-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _XUA_DFU_H_
#define _XUA_DFU_H_ 1

#include <xccompat.h>

#include "xua.h"
#include "xud_device.h"
#include "dfu_types.h"

#ifndef DFU_VENDOR_ID
#error DFU_VENDOR_ID not defined!
#endif

#ifndef DFU_PID
#error DFU_PID not defined!
#endif

#ifndef DFU_PRODUCT_STR_INDEX
#error DFU_PRODUCT_STR_INDEX not defined!!
#endif

#ifndef DFU_MANUFACTURER_STR_INDEX
#error DFU_MANUFACTURER_STR_INDEX not defined!!
#endif

#ifndef __XC__

USB_Descriptor_Device_t DFUdevDesc =
{
    .bLength                        = sizeof(USB_Descriptor_Device_t),
    .bDescriptorType                = USB_DESCTYPE_DEVICE,
#if _XUA_ENABLE_BOS_DESC
    .bcdUSB                         = 0x0201,
#else
    .bcdUSB                         = 0x0200,
#endif
    .bDeviceClass                   = 0, /* See interface */
    .bDeviceSubClass                = 0, /* See interface */
    .bDeviceProtocol                = 0, /* See interface */
    .bMaxPacketSize0                = _DFU_TRANSFER_SIZE_BYTES,
    .idVendor                       = DFU_VENDOR_ID,
    .idProduct                      = DFU_PID,
    .bcdDevice                      = BCD_DEVICE,
    .iManufacturer                  = DFU_MANUFACTURER_STR_INDEX,
    .iProduct                       = DFU_PRODUCT_STR_INDEX,
    .iSerialNumber                  = 0, /* Set to None by default */
    .bNumConfigurations             = 0x01
};

#define DFU_ATTR_CAN_DOWNLOAD              (1u << 0)
#define DFU_ATTR_CAN_UPLOAD                (1u << 1)
#define DFU_ATTR_MANIFESTATION_TOLERANT    (1u << 2)
#define DFU_ATTR_WILL_DETACH               (1u << 3)
// DFU functional attributes
#define DFU_FUNC_ATTRS (DFU_ATTR_CAN_UPLOAD | DFU_ATTR_CAN_DOWNLOAD | DFU_ATTR_WILL_DETACH | DFU_ATTR_MANIFESTATION_TOLERANT)

typedef struct
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned char bmAttributes;
    unsigned short wDetachTimeOut;
    unsigned short wTransferSize;
    unsigned short bcdDFUVersion;
}__attribute__((packed)) USB_DFU_Functional_Descriptor_t;

typedef struct
{
    USB_Descriptor_Configuration_Header_t       Config; /* Configuration header */
    USB_Descriptor_Interface_t                  InterfaceDesc;
    USB_DFU_Functional_Descriptor_t             FunctionalDesc;
}__attribute__((packed)) USB_Config_Descriptor_DFU_t;


USB_Config_Descriptor_DFU_t DFUcfgDesc = {
    .Config =
    {
        .bLength                    = sizeof(USB_Descriptor_Configuration_Header_t),
        .bDescriptorType            = USB_DESCTYPE_CONFIGURATION,
        .wTotalLength               = sizeof(USB_Config_Descriptor_DFU_t),
        .bNumInterfaces             = 1,
        .bConfigurationValue        = 0x01,
        .iConfiguration             = 0x00,
#if (XUA_POWERMODE == XUA_POWERMODE_SELF)
        .bmAttributes               = 192,
#else
        .bmAttributes               = 128,
#endif
        .bMaxPower                  = XUA_BMAX_POWER,
    },
    .InterfaceDesc =
    {
        .bLength                       = sizeof(USB_Descriptor_Interface_t),
        .bDescriptorType               = USB_DESCTYPE_INTERFACE,
        .bInterfaceNumber              = 0,
        .bAlternateSetting             = 0x00,                     /* Must be 0 */
        .bNumEndpoints                 = 0x00,
        .bInterfaceClass               = 0xFE,
        .bInterfaceSubClass            = 0x01,
        .bInterfaceProtocol            = 0x02,
#if (XUA_DFU_EN == 1)
        .iInterface                    = offsetof(StringDescTable_t, dfuStr)/sizeof(char *), /* 8 iInterface */
#else
        .iInterface                    = 0,
#endif
    },
    .FunctionalDesc = {
        .bLength = sizeof(USB_DFU_Functional_Descriptor_t),
        .bDescriptorType = 0x21, //  DFU FUNCTIONAL
        .bmAttributes = DFU_FUNC_ATTRS,
        .wDetachTimeOut = 0x00FA,
        .wTransferSize = _DFU_TRANSFER_SIZE_BYTES,
        .bcdDFUVersion = 0x0110
    }
};

#endif /* __XC__ */

#endif /* _DFU_H_ */

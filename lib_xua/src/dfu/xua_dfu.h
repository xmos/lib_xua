// Copyright 2011-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _XUA_DFU_H_
#define _XUA_DFU_H_ 1

#include <xccompat.h>
#include "xud_device.h"
#include "dfu_types.h"

#ifndef DFU_VENDOR_ID
#error DFU_VENDOR_ID not defined!
#endif

#ifndef DFU_PID
#error DFU_PID not defined!
#endif

#ifndef DFU_PRODUCT_STR_INDEX
#error DFU_PROFUCT_INDEX not defined!!
#endif

#ifndef DFU_MANUFACTURER_STR_INDEX
#error DFU_MANUFACTURE_STR_INDEX not defined!!
#endif



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

unsigned char DFUcfgDesc[] = {
    /* Standard USB device descriptor */
    0x09,                           /* 0  bLength */
    USB_DESCTYPE_CONFIGURATION,              /* 1  bDescriptorType */
    0x1b,                           /* 2  wTotalLength */
    0x00,                           /* 3  wTotalLength */
    1,                              /* 4  bNumInterface: Number of interfaces*/
    0x01,                           /* 5  bConfigurationValue */
    0x00,                           /* 6  iConfiguration */
#if (XUA_POWERMODE == XUA_POWERMODE_SELF)
    192,
#else
    128,
#endif
    _XUA_BMAX_POWER,

    /* Standard DFU class interface descriptor */
    0x09,                           /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                           /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    0x00,                           /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,                           /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,                           /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    0xFE,                           /* 5 bInterfaceClass : AUDIO. (field size 1 bytes) */
    0x01,                           /* 6 bInterfaceSubclass : AUDIO_CONTROL. (field size 1 bytes) */
    0x02,                           /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    offsetof(StringDescTable_t, dfuStr)/sizeof(char *), /* 8 iInterface */

    /* DFU 1.1 Run-Time DFU Functional Descriptor */
    0x09,                           /* 0    Size */
    0x21,                           /* 1    bDescriptorType : DFU FUNCTIONAL */
    DFU_FUNC_ATTRS,                 /* 2    bmAttributes */
    0xFA,                           /* 3    wDetachTimeOut */
    0x00,                           /* 4    wDetachTimeOut */
    0x40,                           /* 5    wTransferSize */
    0x00,                           /* 6    wTransferSize */
    0x10,                           /* 7    bcdDFUVersion */
    0x01,                           /* 8    bcdDFUVersion */
};

int DFUReportResetState(NULLABLE_RESOURCE(chanend , c_user_cmd));
int DFUDeviceRequests(XUD_ep c_ep0_out, NULLABLE_REFERENCE_PARAM(XUD_ep, ep0_in), REFERENCE_PARAM(USB_SetupPacket_t, sp),
        NULLABLE_RESOURCE(chanend, c_user_cmd), unsigned int altInterface, CLIENT_INTERFACE(i_dfu, dfuInterface), REFERENCE_PARAM(int, reset));

/* Helper function for C */
void DFUDelay(unsigned d);

#endif /* _DFU_H_ */

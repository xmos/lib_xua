// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _XUA_DFU_H_
#define _XUA_DFU_H_ 1

#include <xccompat.h>
#include "xud_device.h"

#ifndef DFU_VENDOR_ID
#error DFU_VENDOR_ID not defined!
#endif

#ifndef DFU_PID
#error DFU_PID not defined!
#endif

#ifndef DFU_SERIAL_STR_INDEX
/* By default no serial string */
#define DFU_SERIAL_STR_INDEX        (0x00)
#endif

#ifndef DFU_PRODUCT_STR_INDEX
#error DFU_PROFUCT_INDEX not defined!!
#endif

#ifndef DFU_MANUFACTURER_STR_INDEX
#error DFU_MANUFACTURE_STR_INDEX not defined!!
#endif

unsigned char DFUdevDesc[] = {
    18,                             /* 0  bLength : Size of descriptor in Bytes (18 Bytes) */
    1,                              /* 1  bdescriptorType */
    0,                              /* 2  bcdUSB */
    2,                              /* 3  bcdUSB */
    0x00,                           /* 4  bDeviceClass:      See interface */
    0x00,                           /* 5  bDeviceSubClass:   See interface */
    0,                              /* 6  bDeviceProtocol:   See interface */
    64,                             /* 7  bMaxPacketSize */
    (DFU_VENDOR_ID & 0xFF),         /* 8  idVendor */
    (DFU_VENDOR_ID >> 8),           /* 9  idVendor */
    (DFU_PID & 0xFF),               /* 10 idProduct */
    (DFU_PID >> 8),                 /* 11 idProduct */
    (BCD_DEVICE & 0xFF),        /* 12 bcdDevice : Device release number */
    (BCD_DEVICE >> 8),          /* 13 bcdDevice : Device release number */
    DFU_MANUFACTURER_STR_INDEX,     /* 14 iManufacturer : Index of manufacturer string */
    DFU_PRODUCT_STR_INDEX,          /* 15 iProduct : Index of product string descriptor */
    DFU_SERIAL_STR_INDEX,           /* 16 iSerialNumber : Index of serial number decriptor */
    0x01                            /* 17 bNumConfigurations : Number of possible configs */
};

unsigned char DFUcfgDesc[] = {
    /* Standard USB device descriptor */
    0x09,                           /* 0  bLength */
    USB_DESCTYPE_CONFIGURATION,              /* 1  bDescriptorType */
    0x1b,                           /* 2  wTotalLength */
    0x00,                           /* 3  wTotalLength */
    1,                              /* 4  bNumInterface: Number of interfaces*/
    0x01,                           /* 5  bConfigurationValue */
    0x00,                           /* 6  iConfiguration */
    0xC0,                           /* 7  bmAttributes */
    0x32,                           /* 8  bMaxPower */

    /* Standard DFU class interface descriptor */
    0x09,                           /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                           /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    0x00,                           /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,                           /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,                           /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    0xFE,                           /* 5 bInterfaceClass : AUDIO. (field size 1 bytes) */
    0x01,                           /* 6 bInterfaceSubclass : AUDIO_CONTROL. (field size 1 bytes) */
    0x02,                           /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    0x00,                           /* 8 iInterface : Unused. (field size 1 bytes) */

    /* DFU 1.1 Run-Time DFU Functional Descriptor */
    0x09,                           /* 0    Size */
    0x21,                           /* 1    bDescriptorType : DFU FUNCTIONAL */
    0x07,                           /* 2    bmAttributes */
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


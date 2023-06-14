// Copyright 2011-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
/**
 * @file    xua_ep0_descriptors.h
 * @brief   Device Descriptors
 * @author  Ross Owen, XMOS Limited
*/

#ifndef _DEVICE_DESCRIPTORS_
#define _DEVICE_DESCRIPTORS_

#include <stddef.h>
#include "xua.h"                       /* Device specific define */
#include "descriptor_defs.h"
#include "usbaudio20.h"                /* Defines from the USB Audio 2.0 Specifications */
#include "usbaudiocommon.h"
#include "xud_device.h"
#include "xua_hid_descriptor.h"
#include "xud.h"

#define APPEND_VENDOR_STR(x) VENDOR_STR" "#x

#define APPEND_PRODUCT_STR_A2(x) PRODUCT_STR_A2 " "#x

#define APPEND_PRODUCT_STR_A1(x) PRODUCT_STR_A1 " "#x

#define STR_TABLE_ENTRY(name) char * name

// The empty strings below are used in the g_strTable to set the maximum size of the table entries
// The last char of the strings are different, so that the compiler allocates separate memory spaces
#define XUA_VENDOR_EMPTY_STRING "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\01"
#define XUA_PRODUCT_EMPTY_STRING "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\02"
#define XUA_CLOCK_SELECTOR_EMPTY_STRING "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\03"
#define XUA_INTERNAL_CLOCK_SELECTOR_EMPTY_STRING "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\04"
#define XUA_SPDIF_CLOCK_SOURCE_EMPTY_STRING "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\05"
#define XUA_ADAT_CLOCK_SOURCE_EMPTY_STRING "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\06"
#define XUA_DFU_EMPTY_STRING "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\07"
#define XUA_CTRL_EMPTY_STRING "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\08"
#define XUA_MIDI_OUT_EMPTY_STRING "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\09"
#define XUA_MIDI_IN_EMPTY_STRING "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0a"
#define XUA_SERIAL_EMPTY_STRING "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0b"

// The value below must match the length of XUA_DESCR_EMPTY_STRING.
#define XUA_MAX_STR_LEN (32)

#define ISO_EP_ATTRIBUTES_ASYNC                    ((USB_ENDPOINT_TRANSTYPE_ISO << USB_ENDPOINT_TRANSTYPE_SHIFT)\
                                                    | (USB_ENDPOINT_SYNCTYPE_ASYNC << USB_ENDPOINT_SYNCTYPE_SHIFT)\
                                                    | (USB_ENDPOINT_USAGETYPE_DATA << USB_ENDPOINT_USAGETYPE_SHIFT))

#define ISO_EP_ATTRIBUTES_ADAPTIVE                 ((USB_ENDPOINT_TRANSTYPE_ISO << USB_ENDPOINT_TRANSTYPE_SHIFT)\
                                                    | (USB_ENDPOINT_SYNCTYPE_ADAPT << USB_ENDPOINT_SYNCTYPE_SHIFT)\
                                                    | (USB_ENDPOINT_USAGETYPE_DATA << USB_ENDPOINT_USAGETYPE_SHIFT))

#define ISO_EP_ATTRIBUTES_SYNC                     ((USB_ENDPOINT_TRANSTYPE_ISO << USB_ENDPOINT_TRANSTYPE_SHIFT)\
                                                    | (USB_ENDPOINT_SYNCTYPE_SYNC << USB_ENDPOINT_SYNCTYPE_SHIFT)\
                                                    | (USB_ENDPOINT_USAGETYPE_DATA << USB_ENDPOINT_USAGETYPE_SHIFT))

#define ISO_EP_IMPL_ATTRIBUTES_ASYNC               ((USB_ENDPOINT_TRANSTYPE_ISO << USB_ENDPOINT_TRANSTYPE_SHIFT)\
                                                    | (USB_ENDPOINT_SYNCTYPE_ASYNC << USB_ENDPOINT_SYNCTYPE_SHIFT)\
                                                    | (USB_ENDPOINT_USAGETYPE_IMPLICIT << USB_ENDPOINT_USAGETYPE_SHIFT))

#define ISO_EP_IMPL_ATTRIBUTES_ADAPT               ((USB_ENDPOINT_TRANSTYPE_ISO << USB_ENDPOINT_TRANSTYPE_SHIFT)\
                                                    | (USB_ENDPOINT_SYNCTYPE_ADAPT << USB_ENDPOINT_SYNCTYPE_SHIFT)\
                                                    | (USB_ENDPOINT_USAGETYPE_IMPLICIT << USB_ENDPOINT_USAGETYPE_SHIFT))

#if __STDC__
typedef struct
{
    STR_TABLE_ENTRY(langID);
    STR_TABLE_ENTRY(vendorStr);
    STR_TABLE_ENTRY(serialStr);

#if (AUDIO_CLASS == 2)
    /* Audio 2.0 Strings */
    STR_TABLE_ENTRY(productStr_Audio2);           /* Product string for Audio 2 */
    STR_TABLE_ENTRY(outputInterfaceStr_Audio2);   /* iInterface for streaming intefaces */
    STR_TABLE_ENTRY(inputInterfaceStr_Audio2);    /* iInterface for streaming intefaces */
    STR_TABLE_ENTRY(usbInputTermStr_Audio2);      /* Users sees as output from host */
    STR_TABLE_ENTRY(usbOutputTermStr_Audio2);     /* User sees as input to host */
#endif
#if  (AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS == 1)
    /* Audio 1.0 Strings */
    STR_TABLE_ENTRY(productStr_Audio1);           /* Product string for Audio 1 */
    STR_TABLE_ENTRY(outputInterfaceStr_Audio1);   /* iInterface for streaming intefaces */
    STR_TABLE_ENTRY(inputInterfaceStr_Audio1);    /* iInterface for streaming intefaces */
    STR_TABLE_ENTRY(usbInputTermStr_Audio1);      /* Users sees as output from host */
    STR_TABLE_ENTRY(usbOutputTermStr_Audio1);     /* User sees as input to host */
#endif
#if (AUDIO_CLASS == 2)
    STR_TABLE_ENTRY(clockSelectorStr);            /* iClockSel */
    STR_TABLE_ENTRY(internalClockSourceStr);      /* iClockSource for internal clock */
#if XUA_SPDIF_RX_EN
    STR_TABLE_ENTRY(spdifClockSourceStr);         /* iClockSource for external S/PDIF clock */
#endif
#if (XUA_ADAT_RX_EN)
    STR_TABLE_ENTRY(adatClockSourceStr);          /* iClockSource for external S/PDIF clock */
#endif
#endif // AUDIO_CLASS == 2
#if (XUA_DFU_EN == 1)
    STR_TABLE_ENTRY(dfuStr);                      /* iInterface for DFU interface */
#endif
#ifdef USB_CONTROL_DESCS
    STR_TABLE_ENTRY(ctrlStr);
#endif
#ifdef MIDI
    STR_TABLE_ENTRY(midiOutStr);                  /* iJack for MIDI Out */
    STR_TABLE_ENTRY(midiInStr);                   /* iJack for MIDI In */
#endif
#if (NUM_USB_CHAN_OUT > 0)
    STR_TABLE_ENTRY(outputChanStr_1);
#endif
#if (NUM_USB_CHAN_OUT > 1)
    STR_TABLE_ENTRY(outputChanStr_2);
#endif
#if (NUM_USB_CHAN_OUT > 2)
    STR_TABLE_ENTRY(outputChanStr_3);
#endif
#if (NUM_USB_CHAN_OUT > 3)
    STR_TABLE_ENTRY(outputChanStr_4);
#endif
#if (NUM_USB_CHAN_OUT > 4)
    STR_TABLE_ENTRY(outputChanStr_5);
#endif
#if (NUM_USB_CHAN_OUT > 5)
    STR_TABLE_ENTRY(outputChanStr_6);
#endif
#if (NUM_USB_CHAN_OUT > 6)
    STR_TABLE_ENTRY(outputChanStr_7);
#endif
#if (NUM_USB_CHAN_OUT > 7)
    STR_TABLE_ENTRY(outputChanStr_8);
#endif
#if (NUM_USB_CHAN_OUT > 8)
    STR_TABLE_ENTRY(outputChanStr_9);
#endif
#if (NUM_USB_CHAN_OUT > 9)
    STR_TABLE_ENTRY(outputChanStr_10);
#endif
#if (NUM_USB_CHAN_OUT > 10)
    STR_TABLE_ENTRY(outputChanStr_11);
#endif
#if (NUM_USB_CHAN_OUT > 11)
    STR_TABLE_ENTRY(outputChanStr_12);
#endif
#if (NUM_USB_CHAN_OUT > 12)
    STR_TABLE_ENTRY(outputChanStr_13);
#endif
#if (NUM_USB_CHAN_OUT > 13)
    STR_TABLE_ENTRY(outputChanStr_14);
#endif
#if (NUM_USB_CHAN_OUT > 14)
    STR_TABLE_ENTRY(outputChanStr_15);
#endif
#if (NUM_USB_CHAN_OUT > 15)
    STR_TABLE_ENTRY(outputChanStr_16);
#endif
#if (NUM_USB_CHAN_OUT > 16)
    STR_TABLE_ENTRY(outputChanStr_17);
#endif
#if (NUM_USB_CHAN_OUT > 17)
    STR_TABLE_ENTRY(outputChanStr_18);
#endif
#if (NUM_USB_CHAN_OUT > 18)
    STR_TABLE_ENTRY(outputChanStr_19);
#endif
#if (NUM_USB_CHAN_OUT > 19)
    STR_TABLE_ENTRY(outputChanStr_20);
#endif
#if (NUM_USB_CHAN_OUT > 20)
    STR_TABLE_ENTRY(outputChanStr_21);
#endif
#if (NUM_USB_CHAN_OUT > 21)
    STR_TABLE_ENTRY(outputChanStr_22);
#endif
#if (NUM_USB_CHAN_OUT > 22)
    STR_TABLE_ENTRY(outputChanStr_23);
#endif
#if (NUM_USB_CHAN_OUT > 23)
    STR_TABLE_ENTRY(outputChanStr_24);
#endif
#if (NUM_USB_CHAN_OUT > 24)
    STR_TABLE_ENTRY(outputChanStr_25);
#endif
#if (NUM_USB_CHAN_OUT > 25)
    STR_TABLE_ENTRY(outputChanStr_26);
#endif
#if (NUM_USB_CHAN_OUT > 26)
    STR_TABLE_ENTRY(outputChanStr_27);
#endif
#if (NUM_USB_CHAN_OUT > 27)
    STR_TABLE_ENTRY(outputChanStr_28);
#endif
#if (NUM_USB_CHAN_OUT > 28)
    STR_TABLE_ENTRY(outputChanStr_29);
#endif
#if (NUM_USB_CHAN_OUT > 29)
    STR_TABLE_ENTRY(outputChanStr_30);
#endif
#if (NUM_USB_CHAN_OUT > 30)
    STR_TABLE_ENTRY(outputChanStr_31);
#endif
#if (NUM_USB_CHAN_OUT > 31)
    STR_TABLE_ENTRY(outputChanStr_32);
#endif

#if (NUM_USB_CHAN_OUT > 32)
#error NUM_USB_CHAN > 32
#endif

#if (NUM_USB_CHAN_IN > 0)
    STR_TABLE_ENTRY(inputChanStr_1);
#endif
#if (NUM_USB_CHAN_IN > 1)
    STR_TABLE_ENTRY(inputChanStr_2);
#endif
#if (NUM_USB_CHAN_IN > 2)
    STR_TABLE_ENTRY(inputChanStr_3);
#endif
#if (NUM_USB_CHAN_IN > 3)
    STR_TABLE_ENTRY(inputChanStr_4);
#endif
#if (NUM_USB_CHAN_IN > 4)
    STR_TABLE_ENTRY(inputChanStr_5);
#endif
#if (NUM_USB_CHAN_IN > 5)
    STR_TABLE_ENTRY(inputChanStr_6);
#endif
#if (NUM_USB_CHAN_IN > 6)
    STR_TABLE_ENTRY(inputChanStr_7);
#endif
#if (NUM_USB_CHAN_IN > 7)
    STR_TABLE_ENTRY(inputChanStr_8);
#endif
#if (NUM_USB_CHAN_IN > 8)
    STR_TABLE_ENTRY(inputChanStr_9);
#endif
#if (NUM_USB_CHAN_IN > 9)
    STR_TABLE_ENTRY(inputChanStr_10);
#endif
#if (NUM_USB_CHAN_IN > 10)
    STR_TABLE_ENTRY(inputChanStr_11);
#endif
#if (NUM_USB_CHAN_IN > 11)
    STR_TABLE_ENTRY(inputChanStr_12);
#endif
#if (NUM_USB_CHAN_IN > 12)
    STR_TABLE_ENTRY(inputChanStr_13);
#endif
#if (NUM_USB_CHAN_IN > 13)
    STR_TABLE_ENTRY(inputChanStr_14);
#endif
#if (NUM_USB_CHAN_IN > 14)
    STR_TABLE_ENTRY(inputChanStr_15);
#endif
#if (NUM_USB_CHAN_IN > 15)
    STR_TABLE_ENTRY(inputChanStr_16);
#endif
#if (NUM_USB_CHAN_IN > 16)
    STR_TABLE_ENTRY(inputChanStr_17);
#endif
#if (NUM_USB_CHAN_IN > 17)
    STR_TABLE_ENTRY(inputChanStr_18);
#endif
#if (NUM_USB_CHAN_IN > 18)
    STR_TABLE_ENTRY(inputChanStr_19);
#endif
#if (NUM_USB_CHAN_IN > 19)
    STR_TABLE_ENTRY(inputChanStr_20);
#endif
#if (NUM_USB_CHAN_IN > 20)
    STR_TABLE_ENTRY(inputChanStr_21);
#endif
#if (NUM_USB_CHAN_IN > 21)
    STR_TABLE_ENTRY(inputChanStr_22);
#endif
#if (NUM_USB_CHAN_IN > 22)
    STR_TABLE_ENTRY(inputChanStr_23);
#endif
#if (NUM_USB_CHAN_IN > 23)
    STR_TABLE_ENTRY(inputChanStr_24);
#endif
#if (NUM_USB_CHAN_IN > 24)
    STR_TABLE_ENTRY(inputChanStr_25);
#endif
#if (NUM_USB_CHAN_IN > 25)
    STR_TABLE_ENTRY(inputChanStr_26);
#endif
#if (NUM_USB_CHAN_IN > 26)
    STR_TABLE_ENTRY(inputChanStr_27);
#endif
#if (NUM_USB_CHAN_IN > 27)
    STR_TABLE_ENTRY(inputChanStr_28);
#endif
#if (NUM_USB_CHAN_IN > 28)
    STR_TABLE_ENTRY(inputChanStr_29);
#endif
#if (NUM_USB_CHAN_IN > 29)
    STR_TABLE_ENTRY(inputChanStr_30);
#endif
#if (NUM_USB_CHAN_IN > 30)
    STR_TABLE_ENTRY(inputChanStr_31);
#endif
#if (NUM_USB_CHAN_IN > 31)
    STR_TABLE_ENTRY(inputChanStr_32);
#endif

#if (NUM_USB_CHAN_IN > 32)
#error NUM_USB_CHAN > 32
#endif

#if (MIXER) && (MAX_MIX_COUNT > 0)
    STR_TABLE_ENTRY(mixOutStr_1);
#endif
#if (MIXER) && (MAX_MIX_COUNT > 1)
    STR_TABLE_ENTRY(mixOutStr_2);
#endif
#if (MIXER) && (MAX_MIX_COUNT > 2)
    STR_TABLE_ENTRY(mixOutStr_3);
#endif
#if (MIXER) && (MAX_MIX_COUNT > 3)
    STR_TABLE_ENTRY(mixOutStr_4);
#endif
#if (MIXER) && (MAX_MIX_COUNT > 4)
    STR_TABLE_ENTRY(mixOutStr_5);
#endif
#if (MIXER) && (MAX_MIX_COUNT > 5)
    STR_TABLE_ENTRY(mixOutStr_6);
#endif
#if (MIXER) && (MAX_MIX_COUNT > 6)
    STR_TABLE_ENTRY(mixOutStr_7);
#endif
#if (MIXER) && (MAX_MIX_COUNT > 7)
    STR_TABLE_ENTRY(mixOutStr_8);
#endif
#ifdef IAP
    STR_TABLE_ENTRY(iAPInterfaceStr);
#endif
#ifdef IAP_EA_NATIVE_TRANS
    STR_TABLE_ENTRY(iAP_EANativeTransport_InterfaceStr);
#endif
} StringDescTable_t;

StringDescTable_t g_strTable =
{
    .langID                      = "\x09\x04", /* US English */
    .vendorStr                   = XUA_VENDOR_EMPTY_STRING,
    .serialStr                   = XUA_SERIAL_EMPTY_STRING,
#if (AUDIO_CLASS == 2)
    .productStr_Audio2           = XUA_PRODUCT_EMPTY_STRING,
    .outputInterfaceStr_Audio2   = XUA_PRODUCT_EMPTY_STRING,
    .inputInterfaceStr_Audio2    = XUA_PRODUCT_EMPTY_STRING,
    .usbInputTermStr_Audio2      = XUA_PRODUCT_EMPTY_STRING,
    .usbOutputTermStr_Audio2     = XUA_PRODUCT_EMPTY_STRING,
#endif
#if (AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS == 1)

    .productStr_Audio1           = XUA_PRODUCT_EMPTY_STRING,
    .outputInterfaceStr_Audio1   = XUA_PRODUCT_EMPTY_STRING,
    .inputInterfaceStr_Audio1    = XUA_PRODUCT_EMPTY_STRING,
    .usbInputTermStr_Audio1      = XUA_PRODUCT_EMPTY_STRING,
    .usbOutputTermStr_Audio1     = XUA_PRODUCT_EMPTY_STRING,
#endif
#if (AUDIO_CLASS == 2)
    .clockSelectorStr            = XUA_CLOCK_SELECTOR_EMPTY_STRING,
    .internalClockSourceStr      = XUA_INTERNAL_CLOCK_SELECTOR_EMPTY_STRING,
#if (XUA_SPDIF_RX_EN)
    .spdifClockSourceStr         = XUA_SPDIF_CLOCK_SOURCE_EMPTY_STRING,
#endif
#if (XUA_ADAT_RX_EN)
    .adatClockSourceStr          = XUA_ADAT_CLOCK_SOURCE_EMPTY_STRING,
#endif
#endif // AUDIO_CLASS == 2
#if (XUA_DFU_EN == 1)
    .dfuStr                      = XUA_DFU_EMPTY_STRING,
#endif
#ifdef USB_CONTROL_DESCS
    .ctrlStr                      = XUA_CTRL_EMPTY_STRING,
#endif
#ifdef MIDI
    .midiOutStr                   = XUA_MIDI_OUT_EMPTY_STRING,
    .midiInStr                    = XUA_MIDI_IN_EMPTY_STRING,
#endif

    #include "chanstrings.h"

#if (NUM_USB_CHAN_OUT > 32)
#error NUM_USB_CHAN_OUT > 32
#endif

#if (NUM_USB_CHAN_IN > 32)
#error NUM_USB_CHAN_IN > 32
#endif

#if (MIXER) && (MAX_MIX_COUNT > 0)
    .mixOutStr_1                 = "Mix 1",
#endif
#if (MIXER) && (MAX_MIX_COUNT > 1)
    .mixOutStr_2                 = "Mix 2",
#endif
#if (MIXER) && (MAX_MIX_COUNT > 2)
    .mixOutStr_3                 = "Mix 3",
#endif
#if (MIXER) && (MAX_MIX_COUNT > 3)
    .mixOutStr_4                 = "Mix 4",
#endif
#if (MIXER) && (MAX_MIX_COUNT > 4)
    .mixOutStr_5                 = "Mix 5",
#endif
#if (MIXER) && (MAX_MIX_COUNT > 5)
    .mixOutStr_6                 = "Mix 6",
#endif
#if (MIXER) && (MAX_MIX_COUNT > 6)
    .mixOutStr_7                 = "Mix 7",
#endif
#if (MIXER) && (MAX_MIX_COUNT > 7)
    .mixOutStr_8                 = "Mix 8",
#endif
#if (MIXER) && (MAX_MIX_COUNT > 8)
#error
#endif
#ifdef IAP
    .iAPInterfaceStr             = "iAP Interface",
#endif
#ifdef IAP_EA_NATIVE_TRANS
    .iAP_EANativeTransport_InterfaceStr = IAP2_EA_NATIVE_TRANS_PROTOCOL_NAME,
#endif
};

/***** Device Descriptors *****/

#if (AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS == 1)
/* Device Descriptor for Audio Class 1.0 (Assumes Full-Speed) */
USB_Descriptor_Device_t devDesc_Audio1 =
{
    .bLength                        = sizeof(USB_Descriptor_Device_t),
    .bDescriptorType                = USB_DESCTYPE_DEVICE,
    .bcdUSB                         = 0x0200,
    .bDeviceClass                   = 0,
    .bDeviceSubClass                = 0,
    .bDeviceProtocol                = 0,
    .bMaxPacketSize0                = 64,
    .idVendor                       = VENDOR_ID,
    .idProduct                      = PID_AUDIO_1,
    .bcdDevice                      = BCD_DEVICE,
    .iManufacturer                  = offsetof(StringDescTable_t, vendorStr)/sizeof(char *),
    .iProduct                       = offsetof(StringDescTable_t, productStr_Audio1)/sizeof(char *),
    .iSerialNumber                  = offsetof(StringDescTable_t, serialStr)/sizeof(char *),
    .bNumConfigurations             = 1
};
#endif

#if (AUDIO_CLASS == 2)
/* Device Descriptor for Audio Class 2.0 (Assumes High-Speed )
 *
 * The use of two configurations dates back to Windows XP (could be SP2). This
 * lacked some standards support and incorrectly parsed the full audio class 2.0
 * descriptor with its IADs (Interface Association Descriptors). The observed
 * behaviour included loading the built-in audio class 1.0 driver on some
 * interfaces (possibly the wrong ones, too), and hanging.
 *
 * Presenting a blank configuration first prevented loading the composite driver
 * and issues arising from it, while still allowing to load a vendor driver on
 * the actual configuration.
 *
 * Recent Windows subsystem can parse our class 2.0 descriptor correctly
 * (certainly Windows 7 and onwards). It may be possible to remove this
 * workaround.
 */
USB_Descriptor_Device_t devDesc_Audio2 =
{
    .bLength                        = sizeof(USB_Descriptor_Device_t),
    .bDescriptorType                = USB_DESCTYPE_DEVICE,
    .bcdUSB                         = 0x0200,
    .bDeviceClass                   = 0xEF,
    .bDeviceSubClass                = 0x02,
    .bDeviceProtocol                = 0x01,
    .bMaxPacketSize0                = 64,
    .idVendor                       = VENDOR_ID,
    .idProduct                      = PID_AUDIO_2,
    .bcdDevice                      = BCD_DEVICE,
    .iManufacturer                  = offsetof(StringDescTable_t, vendorStr)/sizeof(char *),
    .iProduct                       = offsetof(StringDescTable_t, productStr_Audio2)/sizeof(char *),
    .iSerialNumber                  = offsetof(StringDescTable_t, serialStr)/sizeof(char *),
    .bNumConfigurations             = 0x01
};

/* Device Descriptor for Null Device */
unsigned char devDesc_Null[] =
{
    18,                             /* 0  bLength : Size of descriptor in Bytes (18 Bytes) */
    USB_DESCTYPE_DEVICE,            /* 1  bdescriptorType */
    0,                              /* 2  bcdUSB */
    2,                              /* 3  bcdUSB */
    0x0,                            /* 4  bDeviceClass */
    0x0  ,                          /* 5  bDeviceSubClass */
    0x00,                           /* 6  bDeviceProtocol */
    64,                             /* 7  bMaxPacketSize */
    (VENDOR_ID & 0xFF),             /* 8  idVendor */
    (VENDOR_ID >> 8),               /* 9  idVendor */
    (PID_AUDIO_2 & 0xFF),           /* 10 idProduct */
    (PID_AUDIO_2 >> 8),             /* 11 idProduct */
    (BCD_DEVICE & 0xFF),            /* 12 bcdDevice : Device release number */
    (BCD_DEVICE >> 8),              /* 13 bcdDevice : Device release number */
    offsetof(StringDescTable_t, vendorStr)/sizeof(char *),
    offsetof(StringDescTable_t, productStr_Audio2)/sizeof(char *),
    offsetof(StringDescTable_t, serialStr)/sizeof(char *),  /* 16 iSerialNumber : Index of serial number decriptor */
    0x01                            /* 17 bNumConfigurations : Number of possible configs */
};
#endif

/****** Device Qualifier Descriptors *****/

/* Device Qualifier Descriptor for Audio 2.0 device (Use when running at full-speed. Matches audio 2.0 device descriptor) */
unsigned char devQualDesc_Audio2[] =
{
    10,                             /* 0  bLength (10 Bytes) */
    USB_DESCTYPE_DEVICE_QUALIFIER,  /* 1  bDescriptorType */
    0x00,                           /* 2  bcdUSB (Binary Coded Decimal of usb version) */
    0x02,                           /* 3  bcdUSB */
    0xEF,                           /* 4  bDeviceClass */
    0x02,                           /* 5  bDeviceSubClass */
    0x01,                           /* 6  bDeviceProtocol */
    64,                             /* 7  bMaxPacketSize */
    0x01,                           /* 8  bNumConfigurations : Number of possible configs */
    0x00                            /* 9  bReserved (must be zero) */
};

#if (AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS == 1)
/* Device Qualifier Descriptor for running at high-speed (matches audio 1.0 device descriptor) */
unsigned char devQualDesc_Audio1[] =
{
    10,                             /* 0  bLength (10 Bytes) */
    USB_DESCTYPE_DEVICE_QUALIFIER,  /* 1  bDescriptorType */
    0x00,                           /* 2  bcdUSB (Binary Coded Decimal of usb version) */
    0x02,                           /* 3  bcdUSB */
    0x00,                           /* 4  bDeviceClass */
    0x00,                           /* 5  bDeviceSubClass */
    0x00,                           /* 6  bDeviceProtocol */
    64,                             /* 7  bMaxPacketSize */
    0x01,                           /* 8  bNumConfigurations : Number of possible configs */
    0x00                            /* 9  bReserved (must be zero) */
};
#endif

/* Device Qualifier Descriptor for Null Device (Use when running at high-speed) */
unsigned char devQualDesc_Null[] =
{
    10,                             /* 0  bLength (10 Bytes) */
    USB_DESCTYPE_DEVICE_QUALIFIER,  /* 1  bDescriptorType */
    0x00,                           /* 2  bcdUSB (Binary Coded Decimal of usb version) */
    0x02,                           /* 3  bcdUSB */
    0x00,                           /* 4  bDeviceClass */
    0x00,                           /* 5  bDeviceSubClass */
    0x00,                           /* 6  bDeviceProtocol */
    64,                             /* 7  bMaxPacketSize */
    0x01,                           /* 8  bNumConfigurations : Number of possible configs */
    0x00                            /* 9  bReserved (must be zero) */
};


#if (MIXER) && !defined(AUDIO_PATH_XUS) && (MAX_MIX_COUNT > 0)
//#warning Extension units on the audio path are required for mixer.  Enabling them now.
#define AUDIO_PATH_XUS
#endif

#ifdef MIDI
#define MIDI_LENGTH                 (92)
#else
#define MIDI_LENGTH                 (0)
#endif

#if (XUA_DFU_EN == 1)
#define DFU_LENGTH                  (18)
#else
#define DFU_LENGTH                  (0)
#endif

#if (MIXER)
    #define MIX_BMCONTROLS_LEN_TMP      ((MAX_MIX_COUNT * MIX_INPUTS) / 8)

    #if ((MAX_MIX_COUNT * MIX_INPUTS)%8)==0
        #define MIX_BMCONTROLS_LEN          (MIX_BMCONTROLS_LEN_TMP)
    #else
        #define MIX_BMCONTROLS_LEN          (MIX_BMCONTROLS_LEN_TMP+1)
    #endif
    #define MIXER_LENGTH                (13+1+MIX_BMCONTROLS_LEN)
#else
    #define MIXER_LENGTH                (0)
#endif

/* Max packet sizes:
 * Samples per channel. e.g (192000+7999/8000) = 24
 * Must allow 1 sample extra per chan (24 + 1) = 25
 * Multiply by number of channels and bytes      25 * 2 * 4 = 200 bytes
 * TODO Output doesn't get modified by channel count
*/
#define MAX_PACKET_SIZE_MULT_OUT_HS ((((MAX_FREQ+7999)/8000)+1) * NUM_USB_CHAN_OUT)
#define MAX_PACKET_SIZE_MULT_OUT_FS ((((MAX_FREQ_FS+999)/1000)+1) * NUM_USB_CHAN_OUT_FS)

#define HS_STREAM_FORMAT_OUTPUT_1_MAXPACKETSIZE (MAX_PACKET_SIZE_MULT_OUT_HS * HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES)
#define HS_STREAM_FORMAT_OUTPUT_2_MAXPACKETSIZE (MAX_PACKET_SIZE_MULT_OUT_HS * HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES)
#define HS_STREAM_FORMAT_OUTPUT_3_MAXPACKETSIZE (MAX_PACKET_SIZE_MULT_OUT_HS * HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES)

#if (HS_STEAM_FORMAT_OUPUT_1_MAXPACKETSIZE > 1024) || (HS_STEAM_FORMAT_OUPUT_2_MAXPACKETSIZE > 1024) \
    || (HS_STEAM_FORMAT_OUPUT_3_MAXPACKETSIZE > 1024)
#error
#endif

#define FS_STREAM_FORMAT_OUTPUT_1_MAXPACKETSIZE (MAX_PACKET_SIZE_MULT_OUT_FS * FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES)
#define FS_STREAM_FORMAT_OUTPUT_2_MAXPACKETSIZE (MAX_PACKET_SIZE_MULT_OUT_FS * FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES)
#define FS_STREAM_FORMAT_OUTPUT_3_MAXPACKETSIZE (MAX_PACKET_SIZE_MULT_OUT_FS * FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES)

/* Input Packet Sizes: high-speed */

#define MAX_PACKET_SIZE_MULT_INPUT_1_HS  ((((MAX_FREQ+7999)/8000)+1) * HS_STREAM_FORMAT_INPUT_1_CHAN_COUNT)
#define MAX_PACKET_SIZE_MULT_INPUT_2_HS  ((((MAX_FREQ+7999)/8000)+1) * HS_STREAM_FORMAT_INPUT_2_CHAN_COUNT)
#define MAX_PACKET_SIZE_MULT_INPUT_3_HS  ((((MAX_FREQ+7999)/8000)+1) * HS_STREAM_FORMAT_INPUT_3_CHAN_COUNT)

/* TODO SUBSLOT_BYTES shared */
#define HS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE (MAX_PACKET_SIZE_MULT_INPUT_1_HS * HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES)
#define HS_STREAM_FORMAT_INPUT_2_MAXPACKETSIZE (MAX_PACKET_SIZE_MULT_INPUT_1_HS * HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES)
#define HS_STREAM_FORMAT_INPUT_3_MAXPACKETSIZE (MAX_PACKET_SIZE_MULT_INPUT_1_HS * HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES)

#if (HS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE > 1024)
#warning HS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE > 1024
#undef HS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE
#define HS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE 1024
#endif

#if (HS_STREAM_FORMAT_INPUT_2_MAXPACKETSIZE > 1024)
#warning HS_STREAM_FORMAT_INPUT_2_MAXPACKETSIZE > 1024
#undef HS_STREAM_FORMAT_INPUT_2_MAXPACKETSIZE
#define HS_STREAM_FORMAT_INPUT_2_MAXPACKETSIZE 1024
#endif

#if (HS_STREAM_FORMAT_INPUT_3_MAXPACKETSIZE > 1024)
#warning HS_STREAM_FORMAT_INPUT_3_MAXPACKETSIZE > 1024
#undef HS_STREAM_FORMAT_INPUT_3_MAXPACKETSIZE
#define HS_STREAM_FORMAT_INPUT_3_MAXPACKETSIZE 1024
#endif

/* Input Packet Sizes: full-speed */
#define MAX_PACKET_SIZE_MULT_IN_FS  ((((MAX_FREQ_FS+999)/1000)+1) * NUM_USB_CHAN_IN_FS)
#define FS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE (MAX_PACKET_SIZE_MULT_IN_FS * FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES)

#if (NUM_CLOCKS == 1)
#define USB_Descriptor_Audio_ClockSelector_t USB_Descriptor_Audio_ClockSelector_1_t
#elif (NUM_CLOCKS == 2)
#define USB_Descriptor_Audio_ClockSelector_t USB_Descriptor_Audio_ClockSelector_2_t
#elif (NUM_CLOCKS == 3)
#define USB_Descriptor_Audio_ClockSelector_t USB_Descriptor_Audio_ClockSelector_3_t
#endif

typedef struct
{
    /* Class Specific Audio Control Interface Header Descriptor */
    UAC_Descriptor_Interface_AC_t               Audio_ClassControlInterface;
    USB_Descriptor_Audio_ClockSource_t          Audio_ClockSource;
#if (XUA_SPDIF_RX_EN)
    USB_Descriptor_Audio_ClockSource_t          Audio_ClockSource_SPDIF;
#endif
#if (XUA_ADAT_RX_EN)
    USB_Descriptor_Audio_ClockSource_t          Audio_ClockSource_ADAT;
#endif
    USB_Descriptor_Audio_ClockSelector_t        Audio_ClockSelector;
#if (NUM_USB_CHAN_OUT > 0)
    /* Output path */
    USB_Descriptor_Audio_InputTerminal_t        Audio_Out_InputTerminal;
#if (MIXER) && (MAX_MIX_COUNT > 0)
    USB_Descriptor_Audio_ExtensionUnit_t        Audio_Out_ExtensionUnit;
#endif
#if(OUTPUT_VOLUME_CONTROL == 1)
    USB_Descriptor_Audio_FeatureUnit_Out_t      Audio_Out_FeatureUnit;
#endif
    USB_Descriptor_Audio_OutputTerminal_t       Audio_Out_OutputTerminal;
#endif
#if (NUM_USB_CHAN_IN > 0)
    /* Input path */
    USB_Descriptor_Audio_InputTerminal_t        Audio_In_InputTerminal;
#if (MIXER) && (MAX_MIX_COUNT > 0)
    USB_Descriptor_Audio_ExtensionUnit_t        Audio_In_ExtensionUnit;
#endif
#if(INPUT_VOLUME_CONTROL == 1)
    USB_Descriptor_Audio_FeatureUnit_In_t       Audio_In_FeatureUnit;
#endif
    USB_Descriptor_Audio_OutputTerminal_t       Audio_In_OutputTerminal;
#endif
#if (MIXER) && (MAX_MIX_COUNT > 0)
    USB_Descriptor_Audio_ExtensionUnit2_t       Audio_Mix_ExtensionUnit;
    // Currently no struct for mixer unit
    // USB_Descriptor_Audio_MixerUnit_t          Audio_MixerUnit;
    unsigned char configDesc_MixerUnit[MIXER_LENGTH];
#endif
#if (XUA_SPDIF_RX_EN) || (XUA_ADAT_RX_EN)
    /* Interrupt EP */
    USB_Descriptor_Endpoint_t                   Audio_Int_Endpoint;
#endif
} __attribute__((packed)) USB_CfgDesc_Audio2_CS_Control_Int;

typedef struct
{
    /* Configuration header */
    USB_Descriptor_Configuration_Header_t       Config;

    /* Audio Control */
    USB_Descriptor_Interface_Association_t      Audio_InterfaceAssociation;
    USB_Descriptor_Interface_t                  Audio_StdControlInterface;       /* Standard Audio Control Interface Header Descriptor */

    USB_CfgDesc_Audio2_CS_Control_Int           Audio_CS_Control_Int;
#if (NUM_USB_CHAN_OUT > 0)
    /* Audio streaming: Output stream */
    USB_Descriptor_Interface_t                  Audio_Out_StreamInterface_Alt0;  /* Zero bandwith alternative */
    USB_Descriptor_Interface_t                  Audio_Out_StreamInterface_Alt1;
    USB_Descriptor_Audio_Interface_AS_t         Audio_Out_ClassStreamInterface;
    USB_Descriptor_Audio_Format_Type1_t         Audio_Out_Format;
    USB_Descriptor_Endpoint_t                   Audio_Out_Endpoint;
    USB_Descriptor_Audio_Class_AS_Endpoint_t    Audio_Out_ClassEndpoint;
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    USB_Descriptor_Endpoint_t                   Audio_Out_Fb_Endpoint;
#endif
#if (OUTPUT_FORMAT_COUNT > 1)
    USB_Descriptor_Interface_t                  Audio_Out_StreamInterface_Alt2;
    USB_Descriptor_Audio_Interface_AS_t         Audio_Out_ClassStreamInterface_2;
    USB_Descriptor_Audio_Format_Type1_t         Audio_Out_Format_2;
    USB_Descriptor_Endpoint_t                   Audio_Out_Endpoint_2;
    USB_Descriptor_Audio_Class_AS_Endpoint_t    Audio_Out_ClassEndpoint_2;
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    USB_Descriptor_Endpoint_t                   Audio_Out_Fb_Endpoint_2;
#endif
#endif // OUTPUT_FORMAT_COUNT > 1
#if (OUTPUT_FORMAT_COUNT > 2)
    USB_Descriptor_Interface_t                  Audio_Out_StreamInterface_Alt3;
    USB_Descriptor_Audio_Interface_AS_t         Audio_Out_ClassStreamInterface_3;
    USB_Descriptor_Audio_Format_Type1_t         Audio_Out_Format_3;
    USB_Descriptor_Endpoint_t                   Audio_Out_Endpoint_3;
    USB_Descriptor_Audio_Class_AS_Endpoint_t    Audio_Out_ClassEndpoint_3;
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    USB_Descriptor_Endpoint_t                   Audio_Out_Fb_Endpoint_3;
#endif
#endif // OUTPUT_FORMAT_COUNT > 2
#endif // NUM_USB_CHAN_OUT > 0
#if (NUM_USB_CHAN_IN > 0)
    /* Audio Streaming: Input stream */
    USB_Descriptor_Interface_t                  Audio_In_StreamInterface_Alt0;  /* Zero bandwith alternative */
    USB_Descriptor_Interface_t                  Audio_In_StreamInterface_Alt1;
    USB_Descriptor_Audio_Interface_AS_t         Audio_In_ClassStreamInterface;
    USB_Descriptor_Audio_Format_Type1_t         Audio_In_Format;
    USB_Descriptor_Endpoint_t                   Audio_In_Endpoint;
    USB_Descriptor_Audio_Class_AS_Endpoint_t    Audio_In_ClassEndpoint;
#if (INPUT_FORMAT_COUNT > 1)
    USB_Descriptor_Interface_t                  Audio_In_StreamInterface_Alt2;
    USB_Descriptor_Audio_Interface_AS_t         Audio_In_ClassStreamInterface_2;
    USB_Descriptor_Audio_Format_Type1_t         Audio_In_Format_2;
    USB_Descriptor_Endpoint_t                   Audio_In_Endpoint_2;
    USB_Descriptor_Audio_Class_AS_Endpoint_t    Audio_In_ClassEndpoint_2;
#endif
#if (INPUT_FORMAT_COUNT > 2)
    USB_Descriptor_Interface_t                  Audio_In_StreamInterface_Alt3;
    USB_Descriptor_Audio_Interface_AS_t         Audio_In_ClassStreamInterface_3;
    USB_Descriptor_Audio_Format_Type1_t         Audio_In_Format_3;
    USB_Descriptor_Endpoint_t                   Audio_In_Endpoint_3;
    USB_Descriptor_Audio_Class_AS_Endpoint_t    Audio_In_ClassEndpoint_3;
#endif
#endif // NUM_USB_CHAN_IN > 0
#ifdef MIDI
    /* MIDI descriptors currently handled as a single block */
    unsigned char configDesc_Midi[MIDI_LENGTH];
#endif

#if (XUA_DFU_EN == 1)
    /* DFU descriptors currently handled as a single block */
    unsigned char configDesc_DFU[DFU_LENGTH];
#endif

#ifdef IAP
    USB_Descriptor_Interface_t                  iAP_Interface;
    USB_Descriptor_Endpoint_t                   iAP_Out_Endpoint;
    USB_Descriptor_Endpoint_t                   iAP_In_Endpoint;
#ifdef IAP_INT_EP
    USB_Descriptor_Endpoint_t                   iAP_Interrupt_Endpoint;
#endif
#ifdef IAP_EA_NATIVE_TRANS
    USB_Descriptor_Interface_t                  iAP_EANativeTransport_Interface_Alt0;
    USB_Descriptor_Interface_t                  iAP_EANativeTransport_Interface_Alt1;
    USB_Descriptor_Endpoint_t                   iAP_EANativeTransport_Out_Endpoint;
    USB_Descriptor_Endpoint_t                   iAP_EANativeTransport_In_Endpoint;
#endif
#endif // IAP

#if XUA_OR_STATIC_HID_ENABLED
    USB_Descriptor_Interface_t                  HID_Interface;
    USB_HID_Descriptor_t                        HID_Descriptor;
    USB_Descriptor_Endpoint_t                   HID_In_Endpoint;
#if HID_OUT_REQUIRED
    USB_Descriptor_Endpoint_t                   HID_Out_Endpoint;
#endif
#endif

}__attribute__((packed)) USB_Config_Descriptor_Audio2_t;

#if (AUDIO_CLASS == 2)
USB_Config_Descriptor_Audio2_t cfgDesc_Audio2=
{
    .Config =
    {
        .bLength                    = sizeof(USB_Descriptor_Configuration_Header_t),
        .bDescriptorType            = USB_DESCTYPE_CONFIGURATION,
        .wTotalLength               = sizeof(USB_Config_Descriptor_Audio2_t),
        .bNumInterfaces             = INTERFACE_COUNT,
        .bConfigurationValue        = 0x01,
        .iConfiguration             = 0x00,
#if (XUA_POWERMODE == XUA_POWERMODE_SELF)
        .bmAttributes               = 192,
#else
        .bmAttributes               = 128,
#endif
        .bMaxPower                  = _XUA_BMAX_POWER,
    },

    .Audio_InterfaceAssociation =
    {
        .bLength                    = sizeof(USB_Descriptor_Interface_Association_t),
        .bDescriptorType            = USB_DESCTYPE_INTERFACE_ASSOCIATION,
        .bFirstInterface            = 0x00,
        .bInterfaceCount            = AUDIO_INTERFACE_COUNT,
        .bFunctionClass             = AUDIO_FUNCTION,
        .bFunctionSubClass          = FUNCTION_SUBCLASS_UNDEFINED,
        .bFunctionProtocol          = UAC_FUNC_PROTOCOL_AF_VERSION_02_00,
        .iFunction                  = 0x00,
    },

    /* Standard Audio Control Interface Descriptor (Note: Must be first with lowest interface number)r */
    .Audio_StdControlInterface =
    {
        .bLength                       = sizeof(USB_Descriptor_Interface_t),
        .bDescriptorType               = USB_DESCTYPE_INTERFACE,
        .bInterfaceNumber              = INTERFACE_NUMBER_AUDIO_CONTROL,
        .bAlternateSetting             = 0x00,                     /* Must be 0 */
#if (XUA_SPDIF_RX_EN) || (XUA_ADAT_RX_EN)
        .bNumEndpoints                 = 0x01,                    /* 0 or 1 if optional interrupt endpoint is present */
#else
        .bNumEndpoints                 = 0x00,
#endif
        .bInterfaceClass               = USB_CLASS_AUDIO,
        .bInterfaceSubClass            = UAC_INT_SUBCLASS_AUDIOCONTROL,
        .bInterfaceProtocol            = UAC_INT_PROTOCOL_IP_VERSION_02_00,
        .iInterface                    = offsetof(StringDescTable_t, productStr_Audio2)/sizeof(char *),
    },

    .Audio_CS_Control_Int =
    {
         /* Class Specific Audio Control Interface Header Descriptor: */
        .Audio_ClassControlInterface =
        {
            .bLength                   = sizeof(UAC_Descriptor_Interface_AC_t),
            .bDescriptorType           = UAC_CS_DESCTYPE_INTERFACE,
            .bDescriptorSubtype        = UAC_CS_AC_INTERFACE_SUBTYPE_HEADER,
            .bcdADC                    = 0x0200,
            .bCatagory                 = UAC_FUNCTION_IO_BOX,  /*(Primary use of audio function) */
            .wTotalLength              = sizeof(USB_CfgDesc_Audio2_CS_Control_Int),
            .bmControls                = 0x00,                 /* 0:1 Latency Control, 2:7 must be 0 */
        },

        /* Clock Source Descriptor (4.7.2.1) */
        .Audio_ClockSource =
        {
            .bLength                   = sizeof(USB_Descriptor_Audio_ClockSource_t),
            .bDescriptorType           = UAC_CS_DESCTYPE_INTERFACE,
            .bDescriptorSubType        = UAC_CS_AC_INTERFACE_SUBTYPE_CLOCK_SOURCE,
            .bClockID                  = ID_CLKSRC_INT,
            .bmAttributes              =  0x03,                   /* D[1:0] :
                                                                        00: External Clock
                                                                        01: Internal Fixed Clock
                                                                        10: Internal Variable Clock
                                                                        11: Internal Progamable Clock
                                                                     D[2]   : Clock synced to SOF
                                                                     D[7:3] : Reserved (0) */
            .bmControls                = 0x07,                    /*
                                                                    D[1:0] : Clock Freq Control
                                                                    D[3:2] : Clock Validity Control
                                                                    D[7:4] : Reserved (0) */
            .bAssocTerminal            = 0x00,
            .iClockSource              = offsetof(StringDescTable_t, internalClockSourceStr)/sizeof(char *),
        },

#if XUA_SPDIF_RX_EN
         /* Clock Source Descriptor (4.7.2.1) */
        .Audio_ClockSource_SPDIF =
        {
            .bLength                   = sizeof(USB_Descriptor_Audio_ClockSource_t),
            .bDescriptorType           = UAC_CS_DESCTYPE_INTERFACE,
            .bDescriptorSubType        = UAC_CS_AC_INTERFACE_SUBTYPE_CLOCK_SOURCE,
            .bClockID                  = ID_CLKSRC_SPDIF,
            .bmAttributes              =  0x00,                   /* D[1:0] :
                                                                        00: External Clock
                                                                        01: Internal Fixed Clock
                                                                        10: Internal Variable Clock
                                                                        11: Internal Progamable Clock
                                                                     D[2]   : Clock synced to SOF
                                                                     D[7:3] : Reserved (0) */
            .bmControls                = 0x07,                    /*
                                                                    D[1:0] : Clock Freq Control
                                                                    D[3:2] : Clock Validity Control
                                                                    D[7:4] : Reserved (0) */
            .bAssocTerminal            = 0x00,
            .iClockSource              = offsetof(StringDescTable_t, spdifClockSourceStr)/sizeof(char *),
        },
#endif

#if (XUA_ADAT_RX_EN)
         /* Clock Source Descriptor (4.7.2.1) */
        .Audio_ClockSource_ADAT =
        {
            .bLength                   = sizeof(USB_Descriptor_Audio_ClockSource_t),
            .bDescriptorType           = UAC_CS_DESCTYPE_INTERFACE,
            .bDescriptorSubType        = UAC_CS_AC_INTERFACE_SUBTYPE_CLOCK_SOURCE,
            .bClockID                  = ID_CLKSRC_ADAT,
            .bmAttributes              =  0x00,                   /* D[1:0] :
                                                                        00: External Clock
                                                                        01: Internal Fixed Clock
                                                                        10: Internal Variable Clock
                                                                        11: Internal Progamable Clock
                                                                     D[2]   : Clock synced to SOF
                                                                     D[7:3] : Reserved (0) */
            .bmControls                = 0x07,                    /*
                                                                    D[1:0] : Clock Freq Control
                                                                    D[3:2] : Clock Validity Control
                                                                    D[7:4] : Reserved (0) */
            .bAssocTerminal            = 0x00,
            .iClockSource              = offsetof(StringDescTable_t, adatClockSourceStr)/sizeof(char *),
        },
#endif


        /* Clock Selector Descriptor (4.7.2.2) */
        .Audio_ClockSelector =
        {
            .bLength                   = sizeof(USB_Descriptor_Audio_ClockSelector_t),
            .bDescriptorType           = UAC_CS_DESCTYPE_INTERFACE,
            .bDescriptorSubType        = UAC_CS_AC_INTERFACE_SUBTYPE_CLOCK_SELECTOR,
            .bClockID                  = ID_CLKSEL,
            .bNrPins                   = NUM_CLOCKS,
            .baCSourceId[0]            = ID_CLKSRC_INT,             /* baCSourceID */
#if (XUA_SPDIF_RX_EN)
            ID_CLKSRC_SPDIF,           /* baCSourceID */

#endif
#if (XUA_ADAT_RX_EN)
            ID_CLKSRC_ADAT,            /* baCSourceID */
#endif
            .bmControl                 = 0x03,
            .iClockSelector            = offsetof(StringDescTable_t, clockSelectorStr)/sizeof(char *),
        },

#if (NUM_USB_CHAN_OUT > 0)
        /* Input Terminal Descriptor (USB Input Terminal) */
        .Audio_Out_InputTerminal =
        {
            .bLength                   = sizeof(USB_Descriptor_Audio_InputTerminal_t),
            .bDescriptorType           = UAC_CS_DESCTYPE_INTERFACE,
            .bDescriptorSubtype        = UAC_CS_AC_INTERFACE_SUBTYPE_INPUT_TERMINAL,
            .bTerminalID               = ID_IT_USB,
            .wTerminalType             = USB_TERMTYPE_USB_STREAMING,
            .bAssocTerminal            = 0x00,
            .bCSourceID                = ID_CLKSEL,
            .bNrChannels               = NUM_USB_CHAN_OUT,
            .bmChannelConfig           = 0x00000000,                               /* TODO. Set me! */
            .iChannelNames             = offsetof(StringDescTable_t, outputChanStr_1)/sizeof(char *),
            .bmControls                =  0x0000,
            .iTerminal                 = offsetof(StringDescTable_t, usbInputTermStr_Audio2)/sizeof(char *)
        },

#if defined (MIXER) && (MAX_MIX_COUNT > 0)
        .Audio_Out_ExtensionUnit =
        {
            .bLength                   = sizeof(USB_Descriptor_Audio_ExtensionUnit_t),
            .bDescriptorType           = UAC_CS_DESCTYPE_INTERFACE,
            .bDescriptorSubtype        = UAC_CS_AC_INTERFACE_SUBTYPE_EXTENSION_UNIT,
            .bUnitID                   = ID_XU_OUT,
            .wExtensionCode            = 0x00,
            .bNrInPins                 = 1,
            .baSourceID[0]             = ID_IT_USB,
            .bNrChannels               = NUM_USB_CHAN_OUT,
            .bmChannelConfig           = 0x00000000,
            .bmControls                = 0x03,
            .iExtension                = 0
        },
#endif

#if(OUTPUT_VOLUME_CONTROL == 1)
        /* Feature Unit Descriptor */
        .Audio_Out_FeatureUnit =
        {
            .bLength                    = sizeof(USB_Descriptor_Audio_FeatureUnit_Out_t),               /* 0  bLength: 6+(ch + 1)*4 */
            0x24,                           /* 1  bDescriptorType: CS_INTERFACE */
            0x06,                           /* 2  bDescriptorSubType: FEATURE_UNIT */
            FU_USBOUT,                      /* 3  bUnitID */
#if defined (MIXER) && (MAX_MIX_COUNT > 0)
            ID_XU_OUT,                      /* 4  bSourceID */
#else
            ID_IT_USB,                      /* 4  bSourceID */
#endif
            {
#if (NUM_USB_CHAN_OUT > 0)
                0x0000000F,         /* bmaControls(0) : Mute and Volume host read and writable */
                0x0000000F,         /* bmaControls(1) */
#endif
#if (NUM_USB_CHAN_OUT > 1)
                0x0000000F,         /* bmaControls(2) */
#endif
#if (NUM_USB_CHAN_OUT > 2)
                0x0000000F,         /* bmaControls(3) */
#endif
#if (NUM_USB_CHAN_OUT > 3)
                0x0000000F,         /* bmaControls(4) */
#endif
#if (NUM_USB_CHAN_OUT > 4)
                0x0000000F,         /* bmaControls(5) */
#endif
#if (NUM_USB_CHAN_OUT > 5)
                0x0000000F,         /* bmaControls(6) */
#endif
#if (NUM_USB_CHAN_OUT > 6)
                0x0000000F,         /* bmaControls(7) */
#endif
#if (NUM_USB_CHAN_OUT > 7)
                0x0000000F,         /* bmaControls(8) */
#endif
#if (NUM_USB_CHAN_OUT > 8)
                0x0000000F,         /* bmaControls(9) */
#endif
#if (NUM_USB_CHAN_OUT > 9)
                0x0000000F,         /* bmaControls(10) */
#endif
#if (NUM_USB_CHAN_OUT > 10)
                0x0000000F,         /* bmaControls(11) */
#endif
#if (NUM_USB_CHAN_OUT > 11)
                0x0000000F,         /* bmaControls(12) */
#endif
#if (NUM_USB_CHAN_OUT > 12)
                0x0000000F,         /* bmaControls(13) */
#endif
#if (NUM_USB_CHAN_OUT > 13)
                0x0000000F,         /* bmaControls(14) */
#endif
#if (NUM_USB_CHAN_OUT > 14)
                0x0000000F,         /* bmaControls(15) */
#endif
#if (NUM_USB_CHAN_OUT > 15)
                0x0000000F,         /* bmaControls(16) */
#endif
#if (NUM_USB_CHAN_OUT > 16)
                0x0000000F,         /* bmaControls(17) */
#endif
#if (NUM_USB_CHAN_OUT > 17)
                0x0000000F,         /* bmaControls(18) */
#endif
#if (NUM_USB_CHAN_OUT > 18)
                0x0000000F,         /* bmaControls(19) */
#endif
#if (NUM_USB_CHAN_OUT > 19)
                0x0000000F,         /* bmaControls(20) */
#endif
#if (NUM_USB_CHAN_OUT > 20)
                0x0000000F,         /* bmaControls(21) */
#endif
#if (NUM_USB_CHAN_OUT > 21)
                0x0000000F,         /* bmaControls(22) */
#endif
#if (NUM_USB_CHAN_OUT > 22)
                0x0000000F,         /* bmaControls(23) */
#endif
#if (NUM_USB_CHAN_OUT > 23)
                0x0000000F,         /* bmaControls(24) */
#endif
#if (NUM_USB_CHAN_OUT > 24)
                0x0000000F,         /* bmaControls(25) */
#endif
#if (NUM_USB_CHAN_OUT > 25)
                0x0000000F,         /* bmaControls(26) */
#endif
#if (NUM_USB_CHAN_OUT > 26)
                0x0000000F,         /* bmaControls(27) */
#endif
#if (NUM_USB_CHAN_OUT > 27)
                0x0000000F,         /* bmaControls(28) */
#endif
#if (NUM_USB_CHAN_OUT > 28)
                0x0000000F,         /* bmaControls(29) */
#endif
#if (NUM_USB_CHAN_OUT > 29)
                0x0000000F,         /* bmaControls(30) */
#endif
#if (NUM_USB_CHAN_OUT > 30)
                0x0000000F,         /* bmaControls(31) */
#endif
#if (NUM_USB_CHAN_OUT > 31)
                0x0000000F,         /* bmaControls(32) */
#endif

#if (NUM_USB_CHAN_OUT > 32)
#error NUM_USB_CHAN_OUT > 32
#endif
            },
            0,                      /* 60 iFeature */
        },
#endif /* (OUTPUT_VOLUME_CONTROL == 1) */

        /* Output Terminal Descriptor (Audio) */
        .Audio_Out_OutputTerminal =
        {
            0x0C,                                        /* 0  bLength */
            UAC_CS_DESCTYPE_INTERFACE,                   /* 1  bDescriptorType: 0x24 */
            UAC_CS_AC_INTERFACE_SUBTYPE_OUTPUT_TERMINAL, /* 2  bDescriptorSubType: OUTPUT_TERMINAL */
            ID_OT_AUD,                                   /* 3  bTerminalID */
            .wTerminalType              = UAC_TT_OUTPUT_TERMTYPE_SPEAKER,
            0x00,                                        /* 6  bAssocTerminal */
#if (OUTPUT_VOLUME_CONTROL == 1)
            FU_USBOUT,                                   /* 7  bSourceID Connect to analog input feature unit*/
#else
            ID_IT_USB,                                   /* 7  bSourceID Connect to analog input feature unit*/
#endif
            ID_CLKSEL,                                   /* 8  bCSourceUD */
            0x0000,                                      /* 9  bmControls */
            0,                                           /* 11 iTerminal */
        },
#endif /* (NUM_USB_CHAN_OUT > 0) */



#if (NUM_USB_CHAN_IN > 0)
    /* Input Terminal Descriptor (Analogue Input Terminal) */
        .Audio_In_InputTerminal =
        {
            .bLength                   = sizeof(USB_Descriptor_Audio_InputTerminal_t),
            .bDescriptorType           = UAC_CS_DESCTYPE_INTERFACE,
            .bDescriptorSubtype        = UAC_CS_AC_INTERFACE_SUBTYPE_INPUT_TERMINAL,
            .bTerminalID               = ID_IT_AUD,
            .wTerminalType             = UAC_TT_INPUT_TERMTYPE_MICROPHONE,
            .bAssocTerminal            = 0x00,
            .bCSourceID                = ID_CLKSEL,
            .bNrChannels               = NUM_USB_CHAN_IN,
            .bmChannelConfig           = 0x00000000,
            .iChannelNames             = offsetof(StringDescTable_t, inputChanStr_1)/sizeof(char *),
            .bmControls                = 0x0000,
            .iTerminal                 = 0,
        },

#if defined (MIXER) && (MAX_MIX_COUNT > 0)
        .Audio_In_ExtensionUnit =
        {
            .bLength                   = sizeof(USB_Descriptor_Audio_ExtensionUnit_t),
            .bDescriptorType           = UAC_CS_DESCTYPE_INTERFACE,
            .bDescriptorSubtype        = UAC_CS_AC_INTERFACE_SUBTYPE_EXTENSION_UNIT,
            .bUnitID                   = ID_XU_IN,
            .wExtensionCode            = 0x00,
            .bNrInPins                 = 1,
            .baSourceID[0]             = ID_IT_AUD,
            .bNrChannels               = NUM_USB_CHAN_IN,
            .bmChannelConfig           = 0x00000000,
            .bmControls                = 0x03,
            .iExtension                = 0
        },
#endif

#if (INPUT_VOLUME_CONTROL == 1)
        .Audio_In_FeatureUnit =
        {
            .bLength                    = sizeof(USB_Descriptor_Audio_FeatureUnit_In_t),
            UAC_CS_DESCTYPE_INTERFACE,    /* 1  bDescriptorType: CS_INTERFACE */
            UAC_CS_AC_INTERFACE_SUBTYPE_FEATURE_UNIT, /* 2  bDescriptorSubType: FEATURE_UNIT */
            FU_USBIN,                     /* 3  bUnitID */
#if (MIXER) && (MAX_MIX_COUNT > 0)
            ID_XU_IN,                     /* 4  bSourceID */
#else
            ID_IT_AUD,                    /* 4  bSourceID */
#endif
            {
#if (NUM_USB_CHAN_IN > 0)
                0x0000000F,               /* bmaControls(0) : Mute and Volume host read and writable */
                0x0000000F,               /* bmaControls(1) */
#endif
#if (NUM_USB_CHAN_IN > 1)
                0x0000000F,               /* bmaControls(2) */
#endif
#if (NUM_USB_CHAN_IN > 2)
                0x0000000F,               /* bmaControls(3) */
#endif
#if (NUM_USB_CHAN_IN > 3)
                0x0000000F,               /* bmaControls(4) */
#endif
#if (NUM_USB_CHAN_IN > 4)
                0x0000000F,               /* bmaControls(5) */
#endif
#if (NUM_USB_CHAN_IN > 5)
                0x0000000F,               /* bmaControls(6) */
#endif
#if (NUM_USB_CHAN_IN > 6)
                0x0000000F,               /* bmaControls(7) */
#endif
#if (NUM_USB_CHAN_IN > 7)
                0x0000000F,               /* bmaControls(8) */
#endif
#if (NUM_USB_CHAN_IN > 8)
                0x0000000F,               /* bmaControls(9) */
#endif
#if (NUM_USB_CHAN_IN > 9)
                0x0000000F,               /* bmaControls(10) */
#endif
#if (NUM_USB_CHAN_IN > 10)
                0x0000000F,               /* bmaControls(11) */
#endif
#if (NUM_USB_CHAN_IN > 11)
                0x0000000F,               /* bmaControls(12) */
#endif
#if (NUM_USB_CHAN_IN > 12)
                0x0000000F,               /* bmaControls(13) */
#endif
#if (NUM_USB_CHAN_IN > 13)
                0x0000000F,               /* bmaControls(14) */
#endif
#if (NUM_USB_CHAN_IN > 14)
                0x0000000F,               /* bmaControls(15) */
#endif
#if (NUM_USB_CHAN_IN > 15)
                0x0000000F,               /* bmaControls(16) */
#endif
#if (NUM_USB_CHAN_IN > 16)
                0x0000000F,               /* bmaControls(17) */
#endif
#if (NUM_USB_CHAN_IN > 17)
                0x0000000F,               /* bmaControls(18) */
#endif
#if (NUM_USB_CHAN_IN > 18)
                0x0000000F,               /* bmaControls(19) */
#endif
#if (NUM_USB_CHAN_IN > 19)
                0x0000000F,               /* bmaControls(20) */
#endif
#if (NUM_USB_CHAN_IN > 20)
                0x0000000F,               /* bmaControls(21) */
#endif
#if (NUM_USB_CHAN_IN > 21)
                0x0000000F,               /* bmaControls(22) */
#endif
#if (NUM_USB_CHAN_IN > 22)
                0x0000000F,               /* bmaControls(23) */
#endif
#if (NUM_USB_CHAN_IN > 23)
                0x0000000F,               /* bmaControls(24) */
#endif
#if (NUM_USB_CHAN_IN > 24)
                0x0000000F,               /* bmaControls(25) */
#endif
#if (NUM_USB_CHAN_IN > 25)
                0x0000000F,               /* bmaControls(26) */
#endif
#if (NUM_USB_CHAN_IN > 26)
                0x0000000F,               /* bmaControls(27) */
#endif
#if (NUM_USB_CHAN_IN > 27)
                0x0000000F,               /* bmaControls(28) */
#endif
#if (NUM_USB_CHAN_IN > 28)
                0x0000000F,               /* bmaControls(29) */
#endif
#if (NUM_USB_CHAN_IN > 29)
                0x0000000F,               /* bmaControls(30) */
#endif
#if (NUM_USB_CHAN_IN > 30)
                0x0000000F,               /* bmaControls(31) */
#endif
#if (NUM_USB_CHAN_IN > 31)
                0x0000000F,               /* bmaControls(32) */
#endif
#if (NUM_USB_CHAN_IN > 32)
#error NUM_USB_CHAN_IN > 32
#endif
            },
            0,                            /* 60 iFeature */
        },
#endif /* (INPUT_VOLUME_CONTROL == 1) */

        .Audio_In_OutputTerminal =
        {
            /* Output Terminal Descriptor (USB Streaming) */
            .bLength                   = 0x0C,
            .bDescriptorType           = UAC_CS_DESCTYPE_INTERFACE,
            .bDescriptorSubtype        = UAC_CS_AC_INTERFACE_SUBTYPE_OUTPUT_TERMINAL,
            .bTerminalID               = ID_OT_USB,
            .wTerminalType             = USB_TERMTYPE_USB_STREAMING,
            .bAssocTerminal            = 0x00,
#if (INPUT_VOLUME_CONTROL == 1)
            .bSourceID                 = FU_USBIN, /* 7  bSourceID Connect to analog input feature unit*/
#else

            .bSourceID                 = ID_IT_AUD,/* 7  bSourceID Connect to analog input term */
#endif
            .bCSourceID                = ID_CLKSEL,
            .bmControls                = 0x0000,
            .iTerminal                 = offsetof(StringDescTable_t, usbOutputTermStr_Audio2)/sizeof(char *)
        },
#endif /* (NUM_USB_CHAN_IN > 0) */

#if (MIXER) && (MAX_MIX_COUNT > 0)
        /* Extension Unit Descriptor (4.7.2.12) */
        .Audio_Mix_ExtensionUnit =
        {
            .bLength                   = sizeof(USB_Descriptor_Audio_ExtensionUnit2_t),
            .bDescriptorType           = UAC_CS_DESCTYPE_INTERFACE,
            .bDescriptorSubtype        = UAC_CS_AC_INTERFACE_SUBTYPE_EXTENSION_UNIT,
            .bUnitID                   = ID_XU_MIXSEL,
            .wExtensionCode            = 0x00,
            .bNrInPins                 = 2,
            .baSourceID[0]             = ID_IT_USB,
            .baSourceID[1]             = ID_IT_AUD,
            .bNrChannels               = MIX_INPUTS,
            .bmChannelConfig           = 0x00000000,
            .bmControls                = 0x03,
            .iExtension                = 0
        },

        /* Mixer Unit Descriptors */
        /* N = 144 (18 * 8) */
        /* Mixer Unit Bitmap - 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff */
        {
            MIXER_LENGTH,                 /* 0 bLength : 13 + num inputs + bit map (inputs * outputs) */
            UAC_CS_DESCTYPE_INTERFACE,    /* bDescriptorType */
            0x04,                         /* bDescriptorSubtype: MIXER_UNIT */
            ID_MIXER_1,                   /* Mixer unit id */
            0x01,                         /* Number of input pins */
            ID_XU_MIXSEL,                 /* Connected terminal or unit id for input pin */
            MAX_MIX_COUNT,                /* Number of mixer output channels */
            0x00, 0x00, 0x00, 0x00,       /* Spacial location ???? */
            offsetof(StringDescTable_t, mixOutStr_1)/sizeof(char *), /* iChannelNames */
#if MIX_BMCONTROLS_LEN > 0                /* Mixer programmable control bitmap */
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 1
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 2
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 3
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 4
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 5
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 6
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 7
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 8
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 9
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 10
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 11
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 12
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 13
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 14
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 15
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 16
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 17
            0xff,
#endif
#if MIX_BMCONTROLS_LEN > 18
#error unxpected BMCONTROLS_LEN
#endif
            0x00,                         /* bmControls */
            0                             /* Mixer unit string descriptor index */
        },
#endif /* (MIXER) && (MAX_MIX_COUNT > 0) */

#if (XUA_SPDIF_RX_EN) || (XUA_ADAT_RX_EN)
        /* Standard AS Interrupt Endpoint Descriptor (4.8.2.1): */
        .Audio_Int_Endpoint =
        {
            .bLength                        = sizeof(USB_Descriptor_Endpoint_t),
            .bDescriptorType                = USB_DESCTYPE_ENDPOINT,
            .bEndpointAddress               = ENDPOINT_ADDRESS_IN_INTERRUPT,     /* (D7: 0:out, 1:in) */
            .bmAttributes                   = 0x03,     /* (bitmap)  */
            .wMaxPacketSize                 = 6,
            .bInterval                      = 8,
        },
#endif
    }, /* End of .Audio_CS_Control_Int */

#if (NUM_USB_CHAN_OUT > 0)
    /* Zero bandwith alternative 0 */
    /* Standard AS Interface Descriptor (4.9.1) */
    .Audio_Out_StreamInterface_Alt0 =
    {
        0x09,                             /* 0  bLength: (in bytes, 9) */
        USB_DESCTYPE_INTERFACE,           /* 1  bDescriptorType: INTERFACE */
        INTERFACE_NUMBER_AUDIO_OUTPUT,    /* 2  bInterfaceNumber: Number of interface */
        0,                                /* 3  bAlternateSetting */
        0,                                /* 4  bNumEndpoints */
        USB_CLASS_AUDIO,                  /* 5  bInterfaceClass: AUDIO */
        UAC_INT_SUBCLASS_AUDIOSTREAMING,  /* 6  bInterfaceSubClass: AUDIO_STREAMING */
        UAC_INT_PROTOCOL_IP_VERSION_02_00,/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
        4,                                /* 8  iInterface: (Sting index) */
    },

    /* Alternative 1 */
    /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    .Audio_Out_StreamInterface_Alt1 =
    {
        0x09,                             /* 0  bLength: (in bytes, 9) */
        USB_DESCTYPE_INTERFACE,           /* 1  bDescriptorType: INTERFACE */
        INTERFACE_NUMBER_AUDIO_OUTPUT,    /* 2  bInterfaceNumber: Number of interface */
        1,                                /* 3  bAlternateSetting */
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
        2,                                /* 4  bNumEndpoints */
#else
        1,                                /* 4  bNumEndpoints */
#endif
        USB_CLASS_AUDIO,                  /* 5  bInterfaceClass: AUDIO */
        UAC_INT_SUBCLASS_AUDIOSTREAMING,  /* 6  bInterfaceSubClass: AUDIO_STREAMING */
        UAC_INT_PROTOCOL_IP_VERSION_02_00,/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
        4,                                /* 8  iInterface: (Sting index) */
     },
    /* STREAMING_OUTPUT_ALT1_OFFSET: */
    /* Class Specific AS Interface Descriptor */
    .Audio_Out_ClassStreamInterface =
    {
        0x10,                             /* 0  bLength: 16 */
        UAC_CS_DESCTYPE_INTERFACE,        /* 1  bDescriptorType: 0x24 */
        UAC_CS_AS_INTERFACE_SUBTYPE_AS_GENERAL, /* 2  bDescriptorSubType */
        ID_IT_USB,                        /* 3  bTerminalLink (Linked to USB input terminal) */
        0x00,                             /* 4  bmControls */
        UAC_FORMAT_TYPE_I,                /* 5  bFormatType */
        STREAM_FORMAT_OUTPUT_1_DATAFORMAT,/* 6:10  bmFormats (note this is a bitmap) */
        NUM_USB_CHAN_OUT,                 /* 11 bNrChannels */
        0x00000000,                       /* 12:14: bmChannelConfig */
        .iChannelNames                 = offsetof(StringDescTable_t, outputChanStr_1)/sizeof(char *),
    },

    /* Type 1 Format Type Descriptor */
    .Audio_Out_Format =
    {
        .bLength                        = 0x06,
        .bDescriptorType                = UAC_CS_DESCTYPE_INTERFACE,
        .bDescriptorSubtype             = UAC_CS_AS_INTERFACE_SUBTYPE_FORMAT_TYPE,
        .bFormatType                    = UAC_FORMAT_TYPE_I,
        .bSubslotSize                   = HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES,
        .bBitResolution                 = HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS,
    },

    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    .Audio_Out_Endpoint =
    {
        .bLength                        = sizeof(USB_Descriptor_Endpoint_t),
        .bDescriptorType                = USB_DESCTYPE_ENDPOINT,
        .bEndpointAddress               = ENDPOINT_ADDRESS_OUT_AUDIO,
#if (XUA_SYNCMODE == XUA_SYNCMODE_ADAPT)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ADAPTIVE,
#elif (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
        #if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ASYNC,         /* Iso, async, data endpoint */
        #else
        .bmAttributes                   = ISO_EP_IMPL_ATTRIBUTES_ASYNC,    /* Feedback data endpoint */
        #endif
#elif (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_SYNC,
#else
    #error "Bad XUA_SYNCMODE"
#endif
        .wMaxPacketSize                 = HS_STREAM_FORMAT_OUTPUT_1_MAXPACKETSIZE,
        .bInterval                      = 1,
    },

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    .Audio_Out_ClassEndpoint =
    {
        0x08,                             /* 0   bLength */
        UAC_CS_DESCTYPE_ENDPOINT,         /* 1   bDescriptorType */
        0x01,                             /* 2   bDescriptorSubtype */
        0x00,                             /* 3   bmAttributes */
        0x00,                             /* 4   bmControls (Bitmap: Pitch control, over/underun etc) */
        0x02,                             /* 5   bLockDelayUnits: Decoded PCM samples */
        0x0008,                           /* 6:7 bLockDelay */
    },

#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    .Audio_Out_Fb_Endpoint =
    {
        .bLength            = 0x07,
        .bDescriptorType    = USB_DESCTYPE_ENDPOINT,
        .bEndpointAddress   = ENDPOINT_ADDRESS_IN_FEEDBACK,
        .bmAttributes       = 17,         /* (bitmap) */
        .wMaxPacketSize     = 0x0004,
        .bInterval          = 4,          /* Only values <= 1 frame (4) supported by MS */
    },
#endif
#if (OUTPUT_FORMAT_COUNT > 1)
    /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    .Audio_Out_StreamInterface_Alt2 =
    {
        0x09,                             /* 0  bLength: (in bytes, 9) */
        USB_DESCTYPE_INTERFACE,           /* 1  bDescriptorType: INTERFACE */
        INTERFACE_NUMBER_AUDIO_OUTPUT,    /* 2  bInterfaceNumber: Number of interface */
        2,                                /* 3  bAlternateSetting */
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
        2,                                /* 4  bNumEndpoints */
#else
        1,                                /* 4  bNumEndpoints */
#endif
        USB_CLASS_AUDIO,                  /* 5  bInterfaceClass: AUDIO */
        UAC_INT_SUBCLASS_AUDIOSTREAMING,  /* 6  bInterfaceSubClass: AUDIO_STREAMING */
        UAC_INT_PROTOCOL_IP_VERSION_02_00,/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
        4,                                /* 8  iInterface: (Sting index) */
     },

    /* Class Specific AS Interface Descriptor */
    .Audio_Out_ClassStreamInterface_2 =
    {
        0x10,                             /* 0  bLength: 16 */
        UAC_CS_DESCTYPE_INTERFACE,        /* 1  bDescriptorType: 0x24 */
        UAC_CS_AS_INTERFACE_SUBTYPE_AS_GENERAL, /* 2  bDescriptorSubType */
        ID_IT_USB,                        /* 3  bTerminalLink (Linked to USB input terminal) */
        0x00,                             /* 4  bmControls */
        UAC_FORMAT_TYPE_I,                /* 5  bFormatType */
        STREAM_FORMAT_OUTPUT_2_DATAFORMAT,/* 6:10  bmFormats (note this is a bitmap) */
        NUM_USB_CHAN_OUT,                 /* 11 bNrChannels */
        0x00000000,                       /* 12:14: bmChannelConfig */
        .iChannelNames                 = (offsetof(StringDescTable_t, outputChanStr_1)/sizeof(char *)),
    },

    /* Type 1 Format Type Descriptor */
    .Audio_Out_Format_2 =
    {
        0x06,                             /* 0  bLength (in bytes): 6 */
        UAC_CS_DESCTYPE_INTERFACE,        /* 1  bDescriptorType: 0x24 */
        UAC_CS_AS_INTERFACE_SUBTYPE_FORMAT_TYPE,/* 2  bDescriptorSubtype: FORMAT_TYPE */
        UAC_FORMAT_TYPE_I,                /* 3  bFormatType: FORMAT_TYPE_1 */
        .bSubslotSize                  = HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES,
        .bBitResolution                = HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS,
    },
    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    .Audio_Out_Endpoint_2 =
    {
        .bLength                        = sizeof(USB_Descriptor_Endpoint_t),
        .bDescriptorType                = USB_DESCTYPE_ENDPOINT,
        .bEndpointAddress               = ENDPOINT_ADDRESS_OUT_AUDIO,
#if (XUA_SYNCMODE == XUA_SYNCMODE_ADAPT)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ADAPTIVE,
#elif (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    #if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ASYNC,         /* Iso, Async, data endpoint */
    #else
        .bmAttributes                   = ISO_EP_IMPL_ATTRIBUTES_ASYNC,    /* Feedback data endpoint */
    #endif
#elif (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_SYNC,          /* Iso, Sync, data endpoint */
#else
    #error "Bad XUA_SYNCMODE"
#endif
        .wMaxPacketSize                 = HS_STREAM_FORMAT_OUTPUT_2_MAXPACKETSIZE,
        .bInterval                      = 1,
    },

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    .Audio_Out_ClassEndpoint_2 =
    {
        0x08,                             /* 0   bLength */
        UAC_CS_DESCTYPE_ENDPOINT,         /* 1   bDescriptorType */
        0x01,                             /* 2   bDescriptorSubtype */
        0x00,                             /* 3   bmAttributes */
        0x00,                             /* 4   bmControls (Bitmap: Pitch control, over/underun etc) */
        0x02,                             /* 5   bLockDelayUnits: Decoded PCM samples */
        0x0008,                           /* 6:7 bLockDelay */
    },

#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    .Audio_Out_Fb_Endpoint_2 =
    {
        0x07,                             /* 0  bLength: 7 */
        USB_DESCTYPE_ENDPOINT,            /* 1  bDescriptorType: ENDPOINT */
        ENDPOINT_ADDRESS_IN_FEEDBACK,     /* 2  bEndpointAddress (D7: 0:out, 1:in) */
        17,                               /* 3  bmAttributes (bitmap)  */
        0x0004,                           /* 4  wMaxPacketSize */
        4,                                /* 6  bInterval. Only values <= 1 frame (4) supported by MS */
    },
#endif
#endif /* OUTPUT_FORMAT_COUNT > 1 */
#if (OUTPUT_FORMAT_COUNT > 2)
    /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    .Audio_Out_StreamInterface_Alt3 =
    {
        0x09,                             /* 0  bLength: (in bytes, 9) */
        USB_DESCTYPE_INTERFACE,           /* 1  bDescriptorType: INTERFACE */
        INTERFACE_NUMBER_AUDIO_OUTPUT,    /* 2  bInterfaceNumber: Number of interface */
        3,                                /* 3  bAlternateSetting */
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
        2,                                /* 4  bNumEndpoints */
#else
        1,                                /* 4  bNumEndpoints */
#endif
        USB_CLASS_AUDIO,                  /* 5  bInterfaceClass: AUDIO */
        UAC_INT_SUBCLASS_AUDIOSTREAMING,  /* 6  bInterfaceSubClass: AUDIO_STREAMING */
        UAC_INT_PROTOCOL_IP_VERSION_02_00,/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
        4,                                /* 8  iInterface: (Sting index) */
     },

    /* Class Specific AS Interface Descriptor */
    .Audio_Out_ClassStreamInterface_3 =
    {
        0x10,                             /* 0  bLength: 16 */
        UAC_CS_DESCTYPE_INTERFACE,        /* 1  bDescriptorType: 0x24 */
        UAC_CS_AS_INTERFACE_SUBTYPE_AS_GENERAL, /* 2  bDescriptorSubType */
        ID_IT_USB,                        /* 3  bTerminalLink (Linked to USB input terminal) */
        0x00,                             /* 4  bmControls */
        UAC_FORMAT_TYPE_I,                /* 5  bFormatType */
        STREAM_FORMAT_OUTPUT_3_DATAFORMAT,/* 6:10  bmFormats (note this is a bitmap) */
        NUM_USB_CHAN_OUT,                 /* 11 bNrChannels */
        0x00000000,                       /* 12:14: bmChannelConfig */
        .iChannelNames                 = offsetof(StringDescTable_t, outputChanStr_1)/sizeof(char *),
    },

    /* Type 1 Format Type Descriptor */
    .Audio_Out_Format_3 =
    {
        .bLength                       = 0x06,
        .bDescriptorType               = UAC_CS_DESCTYPE_INTERFACE,
        .bDescriptorSubtype            = UAC_CS_AS_INTERFACE_SUBTYPE_FORMAT_TYPE,
        .bFormatType                   = UAC_FORMAT_TYPE_I,
        .bSubslotSize                  = HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES,
        .bBitResolution                = HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS,
    },

    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    .Audio_Out_Endpoint_3 =
    {
        .bLength                       = 0x07,
        .bDescriptorType               = USB_DESCTYPE_ENDPOINT,
        .bEndpointAddress              = ENDPOINT_ADDRESS_OUT_AUDIO,
#if (XUA_SYNCMODE == XUA_SYNCMODE_ADAPT)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ADAPTIVE,
#elif (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    #if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ASYNC,         /* Iso, Async, data endpoint */
    #else
        .bmAttributes                   = ISO_EP_IMPL_ATTRIBUTES_ASYNC,    /* Feedback data endpoint */
    #endif
#elif (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_SYNC,          /* Iso, Sync, data endpoint */
#else
    #error "Bad XUA_SYNCMODE"
#endif
        .wMaxPacketSize                = HS_STREAM_FORMAT_OUTPUT_3_MAXPACKETSIZE,
        .bInterval                     = 1,
    },

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    .Audio_Out_ClassEndpoint_3 =
    {
        .bLength                       = 0x08,
        .bDescriptorType               = UAC_CS_DESCTYPE_ENDPOINT,
        .bDescriptorSubtype            = 0x01,
        .bmAttributes                  = 0x00,
        .bmControls                    = 0x00,                 /* (Bitmap: Pitch control, over/underun etc) */
        .bLockDelayUnits               = 0x02,                 /* Decoded PCM samples */
        .wLockDelay                    = 0x0008,
    },

#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    .Audio_Out_Fb_Endpoint_3 =
    {
        .bLength                       = 0x07,
        .bDescriptorType               = USB_DESCTYPE_ENDPOINT,
        .bEndpointAddress              = ENDPOINT_ADDRESS_IN_FEEDBACK,
        .bmAttributes                  = 17,                   /* (bitmap)  */
        .wMaxPacketSize                = 0x0004,
        .bInterval                     = 4,                    /* Only values <= 1 frame (4) supported by MS */
    },
#endif
#endif /* OUTPUT_FORMAT_COUNT > 2 */
#endif /* OUTPUT */
#if (NUM_USB_CHAN_IN > 0)
    /* Zero bandwith alternative 0 */
    /* Standard AS Interface Descriptor (4.9.1) */
    .Audio_In_StreamInterface_Alt0 =
    {
        .bLength                       = 0x09,
        .bDescriptorType               = USB_DESCTYPE_INTERFACE,
        .bInterfaceNumber              = INTERFACE_NUMBER_AUDIO_INPUT,
        .bAlternateSetting             = 0,
        .bNumEndpoints                 = 0,
        .bInterfaceClass               = USB_CLASS_AUDIO,
        .bInterfaceSubClass            = UAC_INT_SUBCLASS_AUDIOSTREAMING,
        .bInterfaceProtocol            = 0x20,
        .iInterface                    = 5, /* (String index) */
    },

    /* Alternative 1 */
    /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    .Audio_In_StreamInterface_Alt1 =
    {
        .bLength                       = 0x09,
        .bDescriptorType               = USB_DESCTYPE_INTERFACE,
        .bInterfaceNumber              = INTERFACE_NUMBER_AUDIO_INPUT,
        .bAlternateSetting             = 1,
        .bNumEndpoints                 = 1,
        .bInterfaceClass               = USB_CLASS_AUDIO,
        .bInterfaceSubClass            = UAC_INT_SUBCLASS_AUDIOSTREAMING,
        .bInterfaceProtocol            = UAC_INT_PROTOCOL_IP_VERSION_02_00,
        .iInterface                    = 5,     /* (String index) */
    },

    /* Class Specific AS Interface Descriptor */
    .Audio_In_ClassStreamInterface =
    {
        .bLength                       = 0x10,
        .bDescriptorType               = UAC_CS_DESCTYPE_INTERFACE,
        .bDescriptorSubType            = UAC_CS_AS_INTERFACE_SUBTYPE_AS_GENERAL,
        .bTerminalLink                 = ID_OT_USB,
        .bmControls                    = 0x00,
        .bFormatType                   = 0x01,
        .bmFormats                     = UAC_FORMAT_TYPEI_PCM,
        .bNrChannels                   = HS_STREAM_FORMAT_INPUT_1_CHAN_COUNT,
        .bmChannelConfig               = 0x00000000,
        .iChannelNames                 = offsetof(StringDescTable_t, inputChanStr_1)/sizeof(char *),
    },

    /* Type 1 Format Type Descriptor */
    .Audio_In_Format =
    {
        .bLength                       = 6,
        .bDescriptorType               = UAC_CS_DESCTYPE_INTERFACE,
        .bDescriptorSubtype            = UAC_CS_AS_INTERFACE_SUBTYPE_FORMAT_TYPE,
        .bFormatType                   = UAC_FORMAT_TYPE_I,
        .bSubslotSize                  = HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES,    /* Number of bytes per subslot */
        .bBitResolution                = HS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS,
    },

    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    .Audio_In_Endpoint =
    {
        .bLength                       = 0x07,
        .bDescriptorType               = USB_DESCTYPE_ENDPOINT,
        .bEndpointAddress              = ENDPOINT_ADDRESS_IN_AUDIO,
#if (XUA_SYNCMODE == XUA_SYNCMODE_ADAPT)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ADAPTIVE,
#elif (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    #if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ASYNC,         /* Iso, Async, data endpoint */
    #else
        .bmAttributes                   = ISO_EP_IMPL_ATTRIBUTES_ASYNC,    /* Feedback data endpoint */
    #endif
#elif (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_SYNC,          /* Iso, Sync, data endpoint */
#else
    #error "Bad XUA_SYNCMODE"
#endif
        .wMaxPacketSize                = HS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE,
        .bInterval                     = 0x01,
    },

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    .Audio_In_ClassEndpoint =
    {
        .bLength                       = sizeof(USB_Descriptor_Audio_Class_AS_Endpoint_t),
        .bDescriptorType               = UAC_CS_DESCTYPE_ENDPOINT,
        .bDescriptorSubtype            = UAC_CS_ENDPOINT_SUBTYPE_EP_GENERAL,
        .bmAttributes                  = 0x00,
        .bmControls                    = 0x00,
        .bLockDelayUnits               = 0x02,
        .wLockDelay                    = 0x0008,
    },
#if (INPUT_FORMAT_COUNT > 1)
    /* Alternative 2 */
    /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    .Audio_In_StreamInterface_Alt2 =
    {
        .bLength                       = 0x09,
        .bDescriptorType               = USB_DESCTYPE_INTERFACE,
        .bInterfaceNumber              = INTERFACE_NUMBER_AUDIO_INPUT,
        .bAlternateSetting             = 2,
        .bNumEndpoints                 = 1,
        .bInterfaceClass               = USB_CLASS_AUDIO,
        .bInterfaceSubClass            = UAC_INT_SUBCLASS_AUDIOSTREAMING,
        .bInterfaceProtocol            = UAC_INT_PROTOCOL_IP_VERSION_02_00,
        .iInterface                    = 5,     /* (String index) */
    },

    /* Class Specific AS Interface Descriptor */
    .Audio_In_ClassStreamInterface_2 =
    {
        .bLength                       = 0x10,
        .bDescriptorType               = UAC_CS_DESCTYPE_INTERFACE,
        .bDescriptorSubType            = UAC_CS_AS_INTERFACE_SUBTYPE_AS_GENERAL,
        .bTerminalLink                 = ID_OT_USB,
        .bmControls                    = 0x00,
        .bFormatType                   = 0x01,
        .bmFormats                     = UAC_FORMAT_TYPEI_PCM,
        .bNrChannels                   = HS_STREAM_FORMAT_INPUT_2_CHAN_COUNT,
        .bmChannelConfig               = 0x00000000,
        .iChannelNames                 = offsetof(StringDescTable_t, inputChanStr_1)/sizeof(char *),
    },

    /* Type 1 Format Type Descriptor */
    .Audio_In_Format_2 =
    {
        .bLength                       = 6,
        .bDescriptorType               = UAC_CS_DESCTYPE_INTERFACE,
        .bDescriptorSubtype            = UAC_CS_AS_INTERFACE_SUBTYPE_FORMAT_TYPE,
        .bFormatType                   = UAC_FORMAT_TYPE_I,
        .bSubslotSize                  = HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES,    /* TODO SUBSLOT_BYTES currently shared */
        .bBitResolution                = HS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS,  /* TODO RESOLUTION_BITS currently shared */
    },

    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    .Audio_In_Endpoint_2 =
    {
        .bLength                       = 0x07,
        .bDescriptorType               = USB_DESCTYPE_ENDPOINT,
        .bEndpointAddress              = ENDPOINT_ADDRESS_IN_AUDIO,
#if (XUA_SYNCMODE == XUA_SYNCMODE_ADAPT)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ADAPTIVE,
#elif (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    #if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ASYNC,         /* Iso, Async, data endpoint */
    #else
        .bmAttributes                   = ISO_EP_IMPL_ATTRIBUTES_ASYNC,    /* Feedback data endpoint */
    #endif
#elif (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_SYNC,          /* Iso, Sync, data endpoint */
#else
    #error "Bad XUA_SYNCMODE"
#endif
        .wMaxPacketSize                = HS_STREAM_FORMAT_INPUT_2_MAXPACKETSIZE,
        .bInterval                     = 0x01,
    },

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    .Audio_In_ClassEndpoint_2 =
    {
        .bLength                       = sizeof(USB_Descriptor_Audio_Class_AS_Endpoint_t),
        .bDescriptorType               = UAC_CS_DESCTYPE_ENDPOINT,
        .bDescriptorSubtype            = UAC_CS_ENDPOINT_SUBTYPE_EP_GENERAL,
        .bmAttributes                  = 0x00,
        .bmControls                    = 0x00,
        .bLockDelayUnits               = 0x02,
        .wLockDelay                    = 0x0008,
    },
#endif /* (INPUT_FORMAT_COUNT > 1) */
#if (INPUT_FORMAT_COUNT > 2)
    /* Alternative 3 */
    /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    .Audio_In_StreamInterface_Alt3 =
    {
        .bLength                       = 0x09,
        .bDescriptorType               = USB_DESCTYPE_INTERFACE,
        .bInterfaceNumber              = INTERFACE_NUMBER_AUDIO_INPUT,
        .bAlternateSetting             = 3,
        .bNumEndpoints                 = 1,
        .bInterfaceClass               = USB_CLASS_AUDIO,
        .bInterfaceSubClass            = UAC_INT_SUBCLASS_AUDIOSTREAMING,
        .bInterfaceProtocol            = UAC_INT_PROTOCOL_IP_VERSION_02_00,
        .iInterface                    = 5,     /* (String index) */
    },

    /* Class Specific AS Interface Descriptor */
    .Audio_In_ClassStreamInterface_3 =
    {
        .bLength                       = 0x10,
        .bDescriptorType               = UAC_CS_DESCTYPE_INTERFACE,
        .bDescriptorSubType            = UAC_CS_AS_INTERFACE_SUBTYPE_AS_GENERAL,
        .bTerminalLink                 = ID_OT_USB,
        .bmControls                    = 0x00,
        .bFormatType                   = 0x01,
        .bmFormats                     = UAC_FORMAT_TYPEI_PCM,
        .bNrChannels                   = HS_STREAM_FORMAT_INPUT_3_CHAN_COUNT,
        .bmChannelConfig               = 0x00000000,
        .iChannelNames                 = offsetof(StringDescTable_t, inputChanStr_1)/sizeof(char *),
    },

    /* Type 1 Format Type Descriptor */
    .Audio_In_Format_3 =
    {
        .bLength                       = 6,
        .bDescriptorType               = UAC_CS_DESCTYPE_INTERFACE,
        .bDescriptorSubtype            = UAC_CS_AS_INTERFACE_SUBTYPE_FORMAT_TYPE,
        .bFormatType                   = UAC_FORMAT_TYPE_I,
        .bSubslotSize                  = HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES,    /* TODO SUBSLOT_BYTES currently shared */
        .bBitResolution                = HS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS,  /* TODO RESOLUTION_BITS currently shared */
    },

    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    .Audio_In_Endpoint_3 =
    {
        .bLength                       = 0x07,
        .bDescriptorType               = USB_DESCTYPE_ENDPOINT,
        .bEndpointAddress              = ENDPOINT_ADDRESS_IN_AUDIO,
#if (XUA_SYNCMODE == XUA_SYNCMODE_ADAPT)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ADAPTIVE,
#elif (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    #if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_ASYNC,         /* Iso, Async, data endpoint */
    #else
        .bmAttributes                   = ISO_EP_IMPL_ATTRIBUTES_ASYNC,    /* Feedback data endpoint */
    #endif
#elif (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
        .bmAttributes                   = ISO_EP_ATTRIBUTES_SYNC,         /* Iso, Sync, data endpoint */
#else
    #error "Bad XUA_SYNCMODE"
#endif
        .wMaxPacketSize                = HS_STREAM_FORMAT_INPUT_3_MAXPACKETSIZE,
        .bInterval                     = 0x01,
    },

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    .Audio_In_ClassEndpoint_3 =
    {
        .bLength                       = sizeof(USB_Descriptor_Audio_Class_AS_Endpoint_t),
        .bDescriptorType               = UAC_CS_DESCTYPE_ENDPOINT,
        .bDescriptorSubtype            = UAC_CS_ENDPOINT_SUBTYPE_EP_GENERAL,
        .bmAttributes                  = 0x00,
        .bmControls                    = 0x00,
        .bLockDelayUnits               = 0x02,
        .wLockDelay                    = 0x0008,
    },
#endif /* (INPUT_FORMAT_COUNT > 2) */
#endif /* (NUM_USB_CHAN_IN > 0) */

#ifdef MIDI
/* MIDI Descriptors */
/* Table B-3: MIDI Adapter Standard AC Interface Descriptor */
    {0x09,                                /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                                 /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    INTERFACE_NUMBER_MIDI_CONTROL,        /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,                                 /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,                                 /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    0x01,                                 /* 5 bInterfaceClass : AUDIO. (field size 1 bytes) */
    0x01,                                 /* 6 bInterfaceSubclass : AUDIO_CONTROL. (field size 1 bytes) */
    0x00,                                 /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    0x00,                                 /* 8 iInterface : Unused. (field size 1 bytes) */

/* Table B-4: MIDI Adapter Class-specific AC Interface Descriptor */
    0x09,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                                 /* 1 bDescriptorType : 0x24. (field size 1 bytes) */
    0x01,                                 /* 2 bDescriptorSubtype : HEADER subtype. (field size 1 bytes) */
    0x00,                                 /* 3 bcdADC : Revision of class specification - 1.0 (field size 2 bytes) */
    0x01,                                 /* 4 bcdADC */
    0x09,                                 /* 5 wTotalLength : Total size of class specific descriptors. (field size 2 bytes) */
    0x00,                                 /* 6 wTotalLength */
    0x01,                                 /* 7 bInCollection : Number of streaming interfaces. (field size 1 bytes) */
    0x01,                                 /* 8 baInterfaceNr(1) : MIDIStreaming interface 1 belongs to this AudioControl interface */

/* Table B-5: MIDI Adapter Standard MS Interface Descriptor */
    0x09,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                                 /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    INTERFACE_NUMBER_MIDI_STREAM,         /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,                                 /* 3 bAlternateSetting : Index of this alternate setting. (field size 1 bytes) */
    0x02,                                 /* 4 bNumEndpoints : 2 endpoints. (field size 1 bytes) */
    0x01,                                 /* 5 bInterfaceClass : AUDIO. (field size 1 bytes) */
    0x03,                                 /* 6 bInterfaceSubclass : MIDISTREAMING. (field size 1 bytes) */
    0x00,                                 /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    0x00,                                 /* 8 iInterface : Unused. (field size 1 bytes) */

/* Table B-6: MIDI Adapter Class-specific MS Interface Descriptor */
    0x07,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                                 /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
    0x01,                                 /* 2 bDescriptorSubtype : MS_HEADER subtype. (field size 1 bytes) */
    0x00,                                 /* 3 BcdADC : Revision of this class specification. (field size 2 bytes) */
    0x01,                                 /* 4 BcdADC */
    0x41,                                 /* 5 wTotalLength : Total size of class-specific descriptors. (field size 2 bytes) */
    0x00,                                 /* 6 wTotalLength */

/* Table B-7: MIDI Adapter MIDI IN Jack Descriptor (Embedded) */
    0x06,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                                 /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
    0x02,                                 /* 2 bDescriptorSubtype : MIDI_IN_JACK subtype. (field size 1 bytes) */
    0x01,                                 /* 3 bJackType : EMBEDDED. (field size 1 bytes) */
    0x01,                                 /* 4 bJackID : ID of this Jack. (field size 1 bytes) */
    0x00,                                 /* 5 iJack : Unused. (field size 1 bytes) */

/* Table B-8: MIDI Adapter MIDI IN Jack Descriptor (External) */
    0x06,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                                 /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
    0x02,                                 /* 2 bDescriptorSubtype : MIDI_IN_JACK subtype. (field size 1 bytes) */
    0x02,                                 /* 3 bJackType : EXTERNAL. (field size 1 bytes) */
    0x02,                                 /* 4 bJackID : ID of this Jack. (field size 1 bytes) */
    offsetof(StringDescTable_t, midiInStr)/sizeof(char *),            /* 5 iJack : Unused. (field size 1 bytes) */

/* Table B-9: MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */
    0x09,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                                 /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
    0x03,                                 /* 2 bDescriptorSubtype : MIDI_OUT_JACK subtype. (field size 1 bytes) */
    0x01,                                 /* 3 bJackType : EMBEDDED. (field size 1 bytes) */
    0x03,                                 /* 4 bJackID : ID of this Jack. (field size 1 bytes) */
    0x01,                                 /* 5 bNrInputPins : Number of Input Pins of this Jack. (field size 1 bytes) */
    0x02,                                 /* 6 BaSourceID(1) : ID of the Entity to which this Pin is connected. (field size 1 bytes) */
    0x01,                                 /* 7 BaSourcePin(1) : Output Pin number of the Entityt o which this Input Pin is connected. */
    0x00,                                 /* 8 iJack : Unused. (field size 1 bytes) */

/* Table B-10: MIDI Adapter MIDI OUT Jack Descriptor (External) */
    0x09,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                                 /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
    0x03,                                 /* 2 bDescriptorSubtype : MIDI_OUT_JACK subtype. (field size 1 bytes) */
    0x02,                                 /* 3 bJackType : EXTERNAL. (field size 1 bytes) */
    0x04,                                 /* 4 bJackID : ID of this Jack. (field size 1 bytes) */
    0x01,                                 /* 5 bNrInputPins : Number of Input Pins of this Jack. (field size 1 bytes) */
    0x01,                                 /* 6 BaSourceID(1) : ID of the Entity to which this Pin is connected. (field size 1 bytes) */
    0x01,                                 /* 7 BaSourcePin(1) : Output Pin number of the Entity to which this Input Pin is connected. */
    offsetof(StringDescTable_t, midiOutStr)/sizeof(char *),            /* 5 iJack : Unused. (field size 1 bytes) */

/* Table B-11: MIDI Adapter Standard Bulk OUT Endpoint Descriptor */
    0x09,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x05,                                 /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
    ENDPOINT_ADDRESS_OUT_MIDI,            /* 2 bEndpointAddress : OUT Endpoint 3. (field size 1 bytes) */
    0x02,                                 /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
    0x00,                                 /* 4 wMaxPacketSize : 512 bytes per packet. (field size 2 bytes) - has to be 0x200 for compliance*/
    0x02,                                 /* 5 wMaxPacketSize */
    0x00,                                 /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */
    0x00,                                 /* 7 bRefresh : Unused. (field size 1 bytes) */
    0x00,                                 /* 8 bSynchAddress : Unused. (field size 1 bytes) */

/* Table B-12: MIDI Adapter Class-specific Bulk OUT Endpoint Descriptor */
    0x05,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x25,                                 /* 1 bDescriptorType : CS_ENDPOINT descriptor (field size 1 bytes) */
    0x01,                                 /* 2 bDescriptorSubtype : MS_GENERAL subtype. (field size 1 bytes) */
    0x01,                                 /* 3 bNumEmbMIDIJack : Number of embedded MIDI IN Jacks. (field size 1 bytes) */
    0x01,                                 /* 4 BaAssocJackID(1) : ID of the Embedded MIDI IN Jack. (field size 1 bytes) */

/* Table B-13: MIDI Adapter Standard Bulk IN Endpoint Descriptor */
    0x09,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x05,                                 /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
    ENDPOINT_ADDRESS_IN_MIDI,             /* 2 bEndpointAddress : IN Endpoint 3. (field size 1 bytes) */
    0x02,                                 /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
    0x00,                                 /* 4 wMaxPacketSize : 512 bytes per packet. (field size 2 bytes) - has to be 0x200 for compliance*/
    0x02,                                 /* 5 wMaxPacketSize */
    0x00,                                 /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */
    0x00,                                 /* 7 bRefresh : Unused. (field size 1 bytes) */
    0x00,                                 /* 8 bSynchAddress : Unused. (field size 1 bytes) */

/* Table B-14: MIDI Adapter Class-specific Bulk IN Endpoint Descriptor */
    0x05,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x25,                                 /* 1 bDescriptorType : CS_ENDPOINT descriptor (field size 1 bytes) */
    0x01,                                 /* 2 bDescriptorSubtype : MS_GENERAL subtype. (field size 1 bytes) */
    0x01,                                 /* 3 bNumEmbMIDIJack : Number of embedded MIDI OUT Jacks. (field size 1 bytes) */
    0x03,                                 /* 4 BaAssocJackID(1) : ID of the Embedded MIDI OUT Jack. (field size 1 bytes) */
    },
#endif // MIDI

#if (XUA_DFU_EN == 1)
    /* Standard DFU class Interface descriptor */
    {0x09,                                /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                                 /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    INTERFACE_NUMBER_DFU,                 /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,                                 /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,                                 /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    0xFE,                                 /* 5 bInterfaceClass : DFU. (Application Specific) (field size 1 bytes) */
    0x01,                                 /* 6 bInterfaceSubclass : (field size 1 bytes) */
    0x01,                                 /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    offsetof(StringDescTable_t, dfuStr)/sizeof(char *), /* 8 iInterface */

#if 0
    /* DFU 1.0 Run-Time DFU Functional Descriptor */
    0x07,
    0x21,
    0x07,
    0xFA,
    0x00,
    0x40,
    0x00
#else
    /* DFU 1.1 Run-Time DFU Functional Descriptor */
    0x09,                                 /* 0    Size */
    0x21,                                 /* 1    bDescriptorType : DFU FUNCTIONAL */
    0x07,                                 /* 2    bmAttributes */
    DFU_DETACH_TIME_OUT & 0xFF,           /* 3    wDetachTimeOut */
    (DFU_DETACH_TIME_OUT >> 8) & 0xFF,    /* 4    wDetachTimeOut */
    0x40,                                 /* 5    wTransferSize */
    0x00,                                 /* 6    wTransferSize */
    0x10,                                 /* 7    bcdDFUVersion */
    0x01},                                /* 7    bcdDFUVersion */
#endif
#endif /* (XUA_DFU_EN == 1) */

#ifdef IAP
    /* Interface descriptor */
    .iAP_Interface =
    {
        .bLength                       = sizeof(USB_Descriptor_Interface_t),
        .bDescriptorType               = USB_DESCTYPE_INTERFACE,
        .bInterfaceNumber              = INTERFACE_NUMBER_IAP,
        .bAlternateSetting             = 0x00,
#ifdef IAP_INT_EP
        .bNumEndpoints                 = 0x03,
#else
        .bNumEndpoints                 = 0x02,
#endif
        .bInterfaceClass               = USB_CLASS_VENDOR_SPECIFIC,
        .bInterfaceSubClass            = 0xF0,                       /* MFI Accessory (Table 38-1) */
        .bInterfaceProtocol            = 0x00,
        .iInterface                    = offsetof(StringDescTable_t, iAPInterfaceStr)/sizeof(char *),   /* Note, string is important! */
    },

    /* iAP Bulk OUT Endpoint Descriptor */
    .iAP_Out_Endpoint =
    {
        0x07,                             /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
        0x05,                             /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
        ENDPOINT_ADDRESS_OUT_IAP,         /* 2 bEndpointAddress : OUT Endpoint 3. High bit isIn (field size 1 bytes) */
        0x02,                             /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
        0x0200,                           /* 4 wMaxPacketSize : Has to be 0x200 for compliance*/
        0x00,                             /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */
    },

    /* iAP Bulk IN Endpoint Descriptor */
    .iAP_In_Endpoint =
    {
        0x07,                             /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
        0x05,                             /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
        ENDPOINT_ADDRESS_IN_IAP,          /* 2 bEndpointAddress : IN Endpoint 5. (field size 1 bytes) */
        0x02,                             /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
        0x0200,                           /* 4 wMaxPacketSize : Has to be 0x200 for compliance*/
        0x00,                             /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */
    },

#ifdef IAP_INT_EP
    /* iAP Interrupt IN Endpoint Descriptor. Note, its usage is now deprecated */
    .iAP_Interrupt_Endpoint =
    {
        0x07,                             /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
        0x05,                             /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
        ENDPOINT_ADDRESS_IN_IAP_INT,      /* 2 bEndpointAddress : IN Endpoint 6. (field size 1 bytes) */
        0x03,                             /* 3 bmAttributes : Interrupt, not shared. (field size 1 bytes) */
        0x0040,                           /* 4 wMaxPacketSize : 64 bytes per packet. (field size 2 bytes) - has to be 0x40 for compliance*/
        0x08,                             /* 6 bInterval : (2^(bInterval-1))/8 ms. Must be between 4 and 32ms (field size 1 bytes) */
    },
#endif
#ifdef IAP_EA_NATIVE_TRANS
    /* iAP EA Native Transport Interface descriptor */
    /* Zero bandwidth alternative 0 */
    .iAP_EANativeTransport_Interface_Alt0 =
    {
        .bLength                    = sizeof(USB_Descriptor_Interface_t),
        .bDescriptorType            = USB_DESCTYPE_INTERFACE,
        .bInterfaceNumber           = INTERFACE_NUMBER_IAP_EA_NATIVE_TRANS,
        .bAlternateSetting          = 0x00,
        .bNumEndpoints              = 0x00,
        .bInterfaceClass            = USB_CLASS_VENDOR_SPECIFIC,
        .bInterfaceSubClass         = 0xF0,                     /* MFI Accessory (Table 21-2) */
        .bInterfaceProtocol         = 0x01,
        .iInterface                 = offsetof(StringDescTable_t, iAP_EANativeTransport_InterfaceStr)/sizeof(char *),
    },

    /* Alternative 1 */
    .iAP_EANativeTransport_Interface_Alt1 =
    {
        .bLength                    = sizeof(USB_Descriptor_Interface_t),
        .bDescriptorType            = USB_DESCTYPE_INTERFACE,
        .bInterfaceNumber           = INTERFACE_NUMBER_IAP_EA_NATIVE_TRANS,
        .bAlternateSetting          = 0x01,
        .bNumEndpoints              = 0x02,
        .bInterfaceClass            = USB_CLASS_VENDOR_SPECIFIC,
        .bInterfaceSubClass         = 0xF0,                     /* MFI Accessory (Table 21-1) */
        .bInterfaceProtocol         = 0x01,
        .iInterface                 = offsetof(StringDescTable_t, iAP_EANativeTransport_InterfaceStr)/sizeof(char *),
    },

    /* iAP EA Native Transport Bulk OUT Endpoint Descriptor */
    .iAP_EANativeTransport_Out_Endpoint =
    {
        0x07,                             /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
        0x05,                             /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
        ENDPOINT_ADDRESS_OUT_IAP_EA_NATIVE_TRANS,  /* 2 bEndpointAddress : OUT Endpoint 3. High bit isIn (field size 1 bytes) */
        0x02,                             /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
        0x0200,                           /* 4 wMaxPacketSize : Has to be 0x200 for compliance*/
        0x00,                             /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */
    },

    /* iAP EA Native Transport Bulk IN Endpoint Descriptor */
    .iAP_EANativeTransport_In_Endpoint =
    {
        0x07,                             /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
        0x05,                             /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
        ENDPOINT_ADDRESS_IN_IAP_EA_NATIVE_TRANS,  /* 2 bEndpointAddress : OUT Endpoint 3. High bit isIn (field size 1 bytes) */
        0x02,                             /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
        0x0200,                           /* 4 wMaxPacketSize : Has to be 0x200 for compliance*/
        0x00,                             /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */
    },
#endif
#endif /* IAP */

#if XUA_OR_STATIC_HID_ENABLED
    #include "xua_hid_descriptors.h"
#endif

};
#endif /* (AUDIO_CLASS == 2) */

#if XUA_OR_STATIC_HID_ENABLED
#if (AUDIO_CLASS ==1 )
unsigned char hidDescriptor[] =
{
    #include "xua_hid_descriptor_contents.h"
};
#elif (AUDIO_CLASS == 2)
unsigned char* hidDescriptor = (unsigned char*) &cfgDesc_Audio2.HID_Descriptor;
#else
    #error "Unknown Audio Class"
#endif
#endif // 0 < HID_CONTROLS


/* Configuration Descriptor for Null device */
unsigned char cfgDesc_Null[] =
{
    0x09,                                 /* 0  bLength */
    USB_DESCTYPE_CONFIGURATION,           /* 1  bDescriptorType */
    0x12,                                 /* 2  wTotalLength */
    0x00,                                 /* 3  wTotalLength */
    0x01,                                 /* 4  bNumInterface: Number of interfaces*/
    0x01,                                 /* 5  bConfigurationValue */
    0x00,                                 /* 6  iConfiguration */
#if (XUA_POWERMODE == XUA_POWERMODE_SELF)
    192,                                  /* 7  bmAttributes */
#else
    128,
#endif
    _XUA_BMAX_POWER,                           /* 8  bMaxPower */

    0x09,                                 /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                                 /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    0x00,                                 /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,                                 /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,                                 /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    0x00,                                 /* 5 bInterfaceClass :  */
    0x00,                                 /* 6 bInterfaceSubclass */
    0x00,                                 /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    0x00,                                 /* 8 iInterface : Unused. (field size 1 bytes) */
    0x09,                                 /* 0 bLength */
};


#if (AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS == 1)
/* Configuration descriptor for Audio v1.0 */
/* Note Audio 1.0 descriptors still a simple array so we need some extra defines regarding lengths.. */
#if (NUM_USB_CHAN_IN > 0)
#define INPUT_INTERFACES_A1         (1)
#else
#define INPUT_INTERFACES_A1         (0)
#endif

#if (NUM_USB_CHAN_OUT > 0)
#define OUTPUT_INTERFACES_A1        (1)
#else
#define OUTPUT_INTERFACES_A1        (0)
#endif

#define AC_LENGTH                   (8 + INPUT_INTERFACES_A1 + OUTPUT_INTERFACES_A1)

/* In UAC1 supported sample rates are listed in descriptor
 * Note, using a value of <=2 or > 7 for num_freqs_a1 causes enumeration issues on Windows.
 * To work around this we repeat MAX_FREQ_FS multiple times in some cases */

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

const unsigned num_freqs_a1 = MAX(3, (0
#if(MIN_FREQ <= 8000) && (MAX_FREQ_FS >= 8000)
    + 1
#endif
#if(MIN_FREQ <= 11025) && (MAX_FREQ_FS >= 11025)
    +1
#endif

#if(MIN_FREQ <= 12000) && (MAX_FREQ_FS >= 12000)
    +1
#endif
#if(MIN_FREQ <= 16000) && (MAX_FREQ_FS >= 16000)
    +1
#endif
#if(MIN_FREQ <= 32000) && (MAX_FREQ_FS >= 32000)
    +1
#endif
#if (MIN_FREQ <= 44100) && (MAX_FREQ_FS >= 44100)
    +1
#endif
#if (MIN_FREQ <= 48000) && (MAX_FREQ_FS >= 48000)
    +1
#endif
#if (MIN_FREQ <= 88200) && (MAX_FREQ_FS >= 88200)
    +1
#endif
#if (MIN_FREQ <= 96000) && (MAX_FREQ_FS >= 96000)
    +1
#endif
));

#ifdef USB_CONTROL_DESCS
#define CONTROL_INTERFACE_BYTES 9
#define NUM_CONTROL_USB_INTERFACES 1
#else
#define CONTROL_INTERFACE_BYTES 0
#define NUM_CONTROL_USB_INTERFACES 0
#endif

#if (XUA_DFU_EN == 1) && (FORCE_UAC1_DFU == 1)
#define DFU_INTERFACE_BYTES   18
#define DFU_INTERFACES_A1     1
#else
#define DFU_INTERFACE_BYTES   0
#define DFU_INTERFACES_A1     0
#endif

#if XUA_OR_STATIC_HID_ENABLED
/*
 * The value of HID_INTERFACE_BYTES must match the length of the descriptors defined in
 * - xua_hid_descriptor_contents.h
 * - xua_hid_endpoint_descriptor_contents.h and
 * - xua_hid_interface_descriptor_contents.h
 */
#define HID_INTERFACE_BYTES   ( 9 + 9 + (7 * (1 + HID_OUT_REQUIRED))) // always IN
#define HID_INTERFACES_A1     1
#else
#define HID_INTERFACE_BYTES   0
#define HID_INTERFACES_A1     0
#endif

/* Total number of bytes returned for the class-specific AudioControl interface descriptor.
 * Includes the combined length of this descriptor header and all Unit and Terminal descriptors
 * For us this is IT -> FU -> OT * 2 and a header */
#define AC_TOTAL_LENGTH             (AC_LENGTH + \
                                    (INPUT_INTERFACES_A1 *  (12 + ( (8 + NUM_USB_CHAN_IN_FS) * INPUT_VOLUME_CONTROL) + 9)) +\
                                    (OUTPUT_INTERFACES_A1 * (12 + ( (8 + NUM_USB_CHAN_OUT_FS) * OUTPUT_VOLUME_CONTROL) + 9)))

#define STREAMING_INTERFACES        (INPUT_INTERFACES_A1 + OUTPUT_INTERFACES_A1)

/* Number of interfaces for Audio  1.0 (+1 for control ) */
/* Note, this is different that INTERFACE_COUNT since we dont support items such as MIDI, iAP etc in UAC1 mode */
#define NUM_INTERFACES_A1           (1 + INPUT_INTERFACES_A1 + OUTPUT_INTERFACES_A1 + NUM_CONTROL_USB_INTERFACES + DFU_INTERFACES_A1 + HID_INTERFACES_A1)

#if ((NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP)) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
#define CFG_TOTAL_LENGTH_A1         (18 + AC_TOTAL_LENGTH + (INPUT_INTERFACES_A1 * (49 + num_freqs_a1 * 3)) + (OUTPUT_INTERFACES_A1 * (58 + num_freqs_a1 * 3)) + CONTROL_INTERFACE_BYTES + DFU_INTERFACE_BYTES + HID_INTERFACE_BYTES)
#else
#define CFG_TOTAL_LENGTH_A1         (18 + AC_TOTAL_LENGTH + (INPUT_INTERFACES_A1 * (49 + num_freqs_a1 * 3)) + (OUTPUT_INTERFACES_A1 * (49 + num_freqs_a1 * 3)) + CONTROL_INTERFACE_BYTES + DFU_INTERFACE_BYTES + HID_INTERFACE_BYTES)
#endif

#define INTERFACE_DESCRIPTOR_BYTES		(9)

#ifdef XUA_USB_DESCRIPTOR_OVERWRITE_RATE_RES
	#define AS_INTERFACE_BYTES		(7)
	#define AS_FORMAT_TYPE_BYTES		(17)
	#define USB_AS_IN_INTERFACE_DESCRIPTOR_OFFSET_SUB_FRAME		(18 + AC_TOTAL_LENGTH + (OUTPUT_INTERFACES_A1 * (49 + num_freqs_a1 * 3)) + (2*INTERFACE_DESCRIPTOR_BYTES) + (AS_INTERFACE_BYTES) + 5)
	#define USB_AS_OUT_INTERFACE_DESCRIPTOR_OFFSET_SUB_FRAME	(18 + AC_TOTAL_LENGTH + (2*INTERFACE_DESCRIPTOR_BYTES) + (AS_INTERFACE_BYTES) + 5)

	#define USB_AS_IN_INTERFACE_DESCRIPTOR_OFFSET_FREQ	(18 + AC_TOTAL_LENGTH + (OUTPUT_INTERFACES_A1 * (49 + num_freqs_a1 * 3)) + (2*INTERFACE_DESCRIPTOR_BYTES) + (AS_INTERFACE_BYTES) + 8)
	#define USB_AS_OUT_INTERFACE_DESCRIPTOR_OFFSET_FREQ	(18 + AC_TOTAL_LENGTH + (2*INTERFACE_DESCRIPTOR_BYTES) + (AS_INTERFACE_BYTES) + 8)

	#define USB_AS_IN_EP_DESCRIPTOR_OFFSET_MAXPACKETSIZE	(18 + AC_TOTAL_LENGTH + (OUTPUT_INTERFACES_A1 * (49 + num_freqs_a1 * 3)) + (2*INTERFACE_DESCRIPTOR_BYTES) + (AS_INTERFACE_BYTES) + (AS_FORMAT_TYPE_BYTES) + 4)
	#define USB_AS_OUT_EP_DESCRIPTOR_OFFSET_MAXPACKETSIZE	(18 + AC_TOTAL_LENGTH + (2*INTERFACE_DESCRIPTOR_BYTES) + (AS_INTERFACE_BYTES) + (AS_FORMAT_TYPE_BYTES) + 4)

#endif

#if XUA_OR_STATIC_HID_ENABLED
    #define USB_HID_DESCRIPTOR_OFFSET (18 + AC_TOTAL_LENGTH + (INPUT_INTERFACES_A1 * (49 + num_freqs_a1 * 3)) + (OUTPUT_INTERFACES_A1 * (49 + num_freqs_a1 * 3)) + CONTROL_INTERFACE_BYTES + DFU_INTERFACE_BYTES + INTERFACE_DESCRIPTOR_BYTES)
#endif

#define CHARIFY_SR(x) (x & 0xff),((x & 0xff00)>> 8),((x & 0xff0000)>> 16)

#if (MIN_FREQ_FS < 12000) && (MAX_FREQ_FS > 48000)
#error SAMPLE RATE RANGE TOO GREAT FOR UAC1 ON WINDOWS
#endif

unsigned char cfgDesc_Audio1[] =
{
    /* Configuration descriptor */
    0x09,
    USB_DESCTYPE_CONFIGURATION,
    (CFG_TOTAL_LENGTH_A1 & 0xFF),         /* wTotalLength */
    (CFG_TOTAL_LENGTH_A1 >> 8),           /* wTotalLength */
    NUM_INTERFACES_A1,                    /* numInterfaces - we dont support MIDI in audio 1.0 mode*/
    0x01,                                 /* ID of this configuration */
    0x00,                                 /* Unused */
#if (XUA_POWERMODE == XUA_POWERMODE_SELF)
    192,                                  /* 7  bmAttributes */
#else
    128,                                  /* 7  bmAttributes */
#endif
    _XUA_BMAX_POWER,                           /* 8  bMaxPower */

    /* Standard AC interface descriptor */
    0x09,
    USB_DESCTYPE_INTERFACE,
    0x00,                                 /* Interface No */
    0x00,                                 /* Alternate setting*/
    0x00,                                 /* Num endpoints */
    USB_CLASS_AUDIO,
    UAC_INT_SUBCLASS_AUDIOCONTROL,
    0x00,                                 /* Unused */
    offsetof(StringDescTable_t, productStr_Audio1)/sizeof(char *), /* iInterface - re-use iProduct */

    /* CS (Class Specific) AudioControl interface header descriptor (4.3.2) */
    AC_LENGTH,
    UAC_CS_DESCTYPE_INTERFACE,
    0x01,                                 /* HEADER */
    0x00, 0x01,                           /* bcdADC */
    (AC_TOTAL_LENGTH & 0xFF),             /* wTotallength (Combined length of this descriptor and all Unit and Terminal Descriptors) */
    (AC_TOTAL_LENGTH >> 8),               /* wTotalLength */
    STREAMING_INTERFACES,                 /* bInCollection (Num streaming interfaces) */
#if (NUM_USB_CHAN_OUT > 0)
    0x01,                                 /* baInterfaceNr(1) AudioStreaming interface 1 belongs to AC interface */
#endif
#if (NUM_USB_CHAN_IN > 0)
    (OUTPUT_INTERFACES_A1 + 1),           /* baInterfaceNr(2) AudioStreaming interface 2 belongs to AC interface */
#endif


#if (NUM_USB_CHAN_OUT > 0)
#define CHANNEL_CONFIG_OUT (0xFF >> (8 - NUM_USB_CHAN_OUT_FS))

    /* CS_Interface Input Terminal 1 Descriptor - USB streaming Host to Device */
    0x0C,
    UAC_CS_DESCTYPE_INTERFACE,            /* bDescriptorType (UAC_CS_DESCTYPE_INTERFACE) */
    0x02,                                 /* bDescriptiorSubtype (INPUT_TERMINAL) */
    0x01,                                 /* bTerminalID */
    0x01, 0x01,                           /* wTerminalType (USB Streaming) */
    0x00,                                 /* Associated terminal - unused  */
    NUM_USB_CHAN_OUT_FS,                  /* bNrChannels */
    CHANNEL_CONFIG_OUT, 0x00,             /* wChannelConfig */
    offsetof(StringDescTable_t, outputChanStr_1)/sizeof(char *), /* iChannelNames */
    offsetof(StringDescTable_t, usbInputTermStr_Audio1)/sizeof(char *), /* iTerminal */

#if (OUTPUT_VOLUME_CONTROL == 1)
    /* CS_Interface class specific AC interface feature unit descriptor - mute & volume for dac */
    (8 + NUM_USB_CHAN_OUT_FS),
    UAC_CS_DESCTYPE_INTERFACE,
    UAC_CS_AC_INTERFACE_SUBTYPE_FEATURE_UNIT, /* 2  bDescriptorSubType: FEATURE_UNIT */
    0x0A,                                 /* unitID */
    0x01,                                 /* sourceID - ID of the unit/terminal to which this feature unit is connected */
    0x01,                                 /* controlSize - 1 */

    0x00,                                 /* bmaControls(0) */
#if (NUM_USB_CHAN_OUT_FS > 0)
    0x03,                                 /* bmaControls(1) */
#endif
#if (NUM_USB_CHAN_OUT_FS > 1)
    0x03,                                 /* bmaControls(2) */
#endif
#if (NUM_USB_CHAN_OUT_FS > 2)
    0x03,                                 /* bmaControls(3) */
#endif
#if (NUM_USB_CHAN_OUT_FS > 3)
    0x03,                                 /* bmaControls(4) */
#endif
#if (NUM_USB_CHAN_OUT_FS > 4)
    0x03,                                 /* bmaControls(5) */
#endif
#if (NUM_USB_CHAN_OUT_FS > 5)
    0x03,                                 /* bmaControls(6) */
#endif
#if (NUM_USB_CHAN_OUT_FS > 6)
    0x03,                                 /* bmaControls(7) */
#endif
#if (NUM_USB_CHAN_OUT_FS > 7)
    0x03,                                 /* bmaControls(8) */
#endif
#if (NUM_USB_CHAN_OUT_FS > 8)
#error NUM_USB_CHAN_OUT_FS > 8 currently supported
#endif
    0x00,                                 /* String table index */
#endif // OUTPUT_VOLUME_CONTROL == 1

    /* CS_Interface Output Terminal Descriptor - Analogue out to speaker */
    0x09,
    UAC_CS_DESCTYPE_INTERFACE,
    0x03,                                 /* OUTPUT_TERMINAL */
    0x06,                                 /* Terminal ID */
    0x01, 0x03,                           /* Type - streaming out, speaker */
    0x00,                                 /* Associated terminal - unused */
#if (OUTPUT_VOLUME_CONTROL == 1)
    0x0A,                                 /* sourceID  - FU */
#else
    0x01,                                 /* sourceID - IT */
#endif
    0x00,                                 /* Unused */
#endif // NUM_USB_CHAN_OUT > 0

#if (NUM_USB_CHAN_IN > 0)
#define CHANNEL_CONFIG_IN (0xFF >> (8 - NUM_USB_CHAN_IN_FS))

    /* CS_Interface Input Terminal 2 Descriptor - Analog in from line in */
    0x0C,
    UAC_CS_DESCTYPE_INTERFACE,
    0x02,                                 /* INPUT_TERMINAL */
    0x02,                                 /* Terminal ID */
    0x01, 0x02,                           /* Type - streaming in, mic */
    0x00,                                 /* Associated terminal - unused  */
    NUM_USB_CHAN_IN_FS,                   /* bNrChannels */
    CHANNEL_CONFIG_IN, 0x00,              /* wChannelConfigs */
    offsetof(StringDescTable_t, inputChanStr_1)/sizeof(char *), /* iChannelNames */
    offsetof(StringDescTable_t, usbOutputTermStr_Audio1)/sizeof(char *), /* iTerminal */

    /* CS_Interface Output Terminal Descriptor - USB Streaming Device to Host*/
    0x09,
    UAC_CS_DESCTYPE_INTERFACE,
    0x03,                                 /* OUTPUT_TERMINAL */
    0x07,                                 /* Terminal ID */
    0x01, 0x01,                           /* Type - streaming */
    0x01,                                 /* Associated terminal - unused */
#if INPUT_VOLUME_CONTROL
    0x0B,                                 /* sourceID - FU */
#else
    0x02,                                 /* sourceID - IT */
#endif
    0x00,                                 /* Unused */

#if (INPUT_VOLUME_CONTROL == 1)
    /* CS_Interface class specific AC interface feature unit descriptor - mute & volume for adc */
    (8 + NUM_USB_CHAN_IN_FS),
    UAC_CS_DESCTYPE_INTERFACE,
    UAC_CS_AC_INTERFACE_SUBTYPE_FEATURE_UNIT, /* 2  bDescriptorSubType: FEATURE_UNIT */
    0x0B,                                 /* unitID */
    0x02,                                 /* sourceID - ID of the unit/terminal to which this feature unit is connected */
    0x01,                                 /* controlSize - 1 */
    0x00,                                 /* bmaControls(0) */
#if (NUM_USB_CHAN_IN_FS > 0)
    0x03,                                 /* bmaControls(1) */
#endif
#if (NUM_USB_CHAN_IN_FS > 1)
    0x03,                                 /* bmaControls(2) */
#endif
#if (NUM_USB_CHAN_IN_FS > 2)
    0x03,                                 /* bmaControls(3) */
#endif
#if (NUM_USB_CHAN_IN_FS > 3)
    0x03,                                 /* bmaControls(4) */
#endif
#if (NUM_USB_CHAN_IN_FS > 4)
    0x03,                                 /* bmaControls(5) */
#endif
#if (NUM_USB_CHAN_IN_FS > 5)
    0x03,                                 /* bmaControls(6) */
#endif
#if (NUM_USB_CHAN_IN_FS > 6)
    0x03,                                 /* bmaControls(7) */
#endif
#if (NUM_USB_CHAN_IN_FS > 7)
    0x03,                                 /* bmaControls(8) */
#endif
#if (NUM_USB_CHAN_IN_FS > 8)
#error NUM_USB_CHAN_IN_FS > 8 currently supported
#endif
    0x00,                                 /* String table index */
#endif // INPUT_VOLUME_CONTROL == 1
#endif // NUM_USB_CHAN_IN > 0

#if (NUM_USB_CHAN_OUT > 0)
    /* Standard AS Interface Descriptor (4.5.1) */
    0x09,                                 /* bLength */
    0x04,                                 /* INTERFACE */
    0x01,                                 /* bInterfaceNumber */
    0x00,                                 /* bAlternateSetting */
    0x00,                                 /* bnumEndpoints */
    0x01,                                 /* bInterfaceClass - AUDIO */
    0x02,                                 /* bInterfaceSubclass - AUDIO_STREAMING */
    0x00,                                 /* bInterfaceProtocol - Not used */
    offsetof(StringDescTable_t, outputInterfaceStr_Audio1)/sizeof(char *), /* iInterface */

    /* Standard As Interface Descriptor (4.5.1) */
    0x09,
    0x04,                                 /* INTERFACE */
    0x01,                                 /* bInterfaceNumber */
    0x01,                                 /* bAlternateSetting */
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    0x02,                                 /* bNumEndpoints 2: audio EP and feedback EP */
#else
    0x01,                                 /* bNumEndpoints */
#endif
    0x01,                                 /* bInterfaceClas - AUDIO */
    0x02,                                 /* bInterfaceSubclass - AUDIO_STREAMING */
    0x00,                                 /* bInterface Protocol - Unused */
    offsetof(StringDescTable_t, outputInterfaceStr_Audio1)/sizeof(char *), /* iInterface */

    /* Class-Specific AS Interface Descriptor (4.5.2) */
    0x07,
    UAC_CS_DESCTYPE_INTERFACE,            /* bDescriptorType */
    0x01,                                 /* bDescriptorSubtype - GENERAL */
    0x01,                                 /* iTerminalLink - linked to Streaming IN terminal */
    0x01,                                 /* bDelay */
    0x01, 0x00,                           /* wFormatTag - PCM */

    /* CS_Interface Format Type Descriptor */
    (8 + (num_freqs_a1 * 3)),
    UAC_CS_DESCTYPE_INTERFACE,
    0x02,                                 /* Subtype - FORMAT_TYPE */
    0x01,                                 /* Format type - FORMAT_TYPE_1 */
    NUM_USB_CHAN_OUT_FS,                  /* nrChannels */
    FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES,   /* subFrameSize */
    FS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS, /* bitResolution */
    num_freqs_a1,                         /* SamFreqType - sample freq count */

/* Windows enum issue with <= two sample rates work around */
#if ((MIN_FREQ == 8000) && (MAX_FREQ_FS == 11025)) \
    || (MIN_FREQ == 11025) && (MAX_FREQ_FS == 12000) \
    || (MIN_FREQ == 12000) && (MAX_FREQ_FS == 16000) \
    || (MIN_FREQ == 16000) && (MAX_FREQ_FS == 32000) \
    || (MIN_FREQ == 44100) && (MAX_FREQ_FS == 48000) \
    || (MIN_FREQ == 48000) && (MAX_FREQ_FS == 88200) \
    || (MIN_FREQ == 88200) && (MAX_FREQ_FS == 96000)
    CHARIFY_SR(MAX_FREQ_FS),
#endif
#if (MIN_FREQ == MAX_FREQ_FS)
    CHARIFY_SR(MAX_FREQ_FS),
    CHARIFY_SR(MAX_FREQ_FS),
#endif

#if(MIN_FREQ <= 8000) && (MAX_FREQ_FS >= 8000)
    0x40, 0x1F, 0x00,                     /* sampleFreq - 8KHz */
#endif

#if(MIN_FREQ <= 11025) && (MAX_FREQ_FS >= 11025)
    0x11, 0x2B, 0x00,                     /* sampleFreq - 11.25KHz */
#endif

#if(MIN_FREQ <= 12000) && (MAX_FREQ_FS >= 12000)
    0xE0, 0x2E, 0x00,                     /* sampleFreq - 12KHz */
#endif

#if(MIN_FREQ <= 16000) && (MAX_FREQ_FS >= 16000)
    CHARIFY_SR(16000),                    /* sampleFreq - 16KHz */
#endif

#if(MIN_FREQ <= 32000) && (MAX_FREQ_FS >= 32000)
    CHARIFY_SR(32000),                    /* sampleFreq - 32KHz */
#endif

#if (MIN_FREQ <= 44100) && (MAX_FREQ_FS >= 44100)
    0x44, 0xAC, 0x00,                     /* sampleFreq - 44.1Khz */
#endif

#if (MIN_FREQ <= 48000) && (MAX_FREQ_FS >= 48000)
    0x80, 0xBB, 0x00,                     /* sampleFreq - 48KHz */
#endif

#if (MIN_FREQ <= 88200) && (MAX_FREQ_FS >= 88200)
    0x88, 0x58, 0x01,                     /* sampleFreq - 88.2KHz */
#endif

#if (MIN_FREQ <= 96000) && (MAX_FREQ_FS >= 96000)
    0x00, 0x77, 0x01,                     /* sampleFreq - 96KHz */
#endif

    /* Standard AS Isochronous Audio Data Endpoint Descriptor 4.6.1.1 */
    0x09,
    0x05,                                 /* ENDPOINT */
    ENDPOINT_ADDRESS_OUT_AUDIO,           /* endpointAddress - D7, direction (0 OUT, 1 IN). D6..4 reserved (0). D3..0 endpoint no. */
#if (XUA_SYNCMODE == XUA_SYNCMODE_ADAPT)
    ISO_EP_ATTRIBUTES_ADAPTIVE,
#elif (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    #if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP)
    ISO_EP_ATTRIBUTES_ASYNC,              /* Iso, async, data endpoint */
    #else
    ISO_EP_IMPL_ATTRIBUTES_ASYNC,         /* Feedback data endpoint */
    #endif
#elif (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
    ISO_EP_ATTRIBUTES_SYNC,               /* Iso, sync, data endpoint */
#else
#error "Unsupported XUA_SYNCMODE"
#endif
    (FS_STREAM_FORMAT_OUTPUT_1_MAXPACKETSIZE&0xff),      /* 4  wMaxPacketSize (Typically 294 bytes)*/
    (FS_STREAM_FORMAT_OUTPUT_1_MAXPACKETSIZE&0xff00)>>8, /* 5  wMaxPacketSize */
    0x01,                                 /* bInterval */
    0x00,                                 /* bRefresh */
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    ENDPOINT_ADDRESS_IN_FEEDBACK,         /* bSynchAdddress - address of EP used to communicate sync info */
#else                                     /* Bi-directional in/out device */
    #if (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
        ENDPOINT_ADDRESS_IN_AUDIO,
    #else
        0,                                /* Unused */
    #endif
#endif

    /* CS_Endpoint Descriptor ?? */
    0x07,
    0x25,                                 /* CS_ENDPOINT */
    0x01,                                 /* subtype - GENERAL */
    0x01,                                 /* attributes. D[0]: sample freq ctrl. */
    0x02,                                 /* bLockDelayUnits */
#if (XUA_SYNCMODE == XUA_SYNCMODE_ADAPT)
    0x08, 0x00,                           /* bLockDelay */
#else
    0x00, 0x00,                           /* Not used */
#endif

#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    /* Feedback EP */
    0x09,
    0x05,                                 /* bDescriptorType: ENDPOINT */
    ENDPOINT_ADDRESS_IN_FEEDBACK,         /* bEndpointAddress (D3:0 - EP no. D6:4 - reserved 0. D7 - 0:out, 1:in) */
    0x01,                                 /* bmAttributes (bitmap)  */
    0x03,0x0,                             /* wMaxPacketSize */
    0x01,                                 /* bInterval - Must be 1 for compliance */
    0x04,                                 /* bRefresh 2^x */
    0x0,                                  /* bSynchAddress */
#endif
#endif // NUM_USB_CHAN_OUT > 0

#if (NUM_USB_CHAN_IN > 0)
    /* Standard Interface Descriptor - Audio streaming IN */
    0x09,
    0x04,                                 /* INTERFACE */
    (OUTPUT_INTERFACES_A1 + 1),           /* bInterfaceNumber*/
    0x00,                                 /* AlternateSetting */
    0x00,                                 /* bNumEndpoints */
    0x01,                                 /* Interface class - AUDIO */
    0x02,                                 /* subclass - AUDIO_STREAMING */
    0x00,                                 /* Unused */
    offsetof(StringDescTable_t, inputInterfaceStr_Audio1)/sizeof(char *),

    /* Standard Interface Descriptor - Audio streaming IN */
    0x09,
    0x04,                                 /* INTERFACE */
    (OUTPUT_INTERFACES_A1 + 1),           /* bInterfaceNumber */
    0x01,                                 /* AlternateSetting */
    0x01,                                 /* bNumEndpoints */
    0x01,                                 /* Interface class - AUDIO */
    0x02,                                 /* Subclass - AUDIO_STREAMING */
    0x00,                                 /* Unused */
    offsetof(StringDescTable_t, inputInterfaceStr_Audio1)/sizeof(char *),

    /* CS_Interface AC interface header descriptor */
    0x07,
    UAC_CS_DESCTYPE_INTERFACE,
    0x01,                                 /* subtype - GENERAL */
    0x07,                                 /* TerminalLink - linked to Streaming OUT terminal */
    0x01,                                 /* Interface delay */
    0x01,0x00,                            /* Format - PCM */

    /* CS_Interface Terminal Descriptor */
    (8 + (num_freqs_a1 * 3)),
    UAC_CS_DESCTYPE_INTERFACE,
    0x02,                                 /* Subtype - FORMAT_TYPE */
    0x01,                                 /* Format type - FORMAT_TYPE_1 */
    NUM_USB_CHAN_IN_FS,                   /* bNrChannels - Typically 2 */
    FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES,         /* subFrameSize - Typically 4 bytes per slot */
    FS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS,       /* bitResolution - Typically 24bit */
    num_freqs_a1,                         /* SamFreqType - sample freq count */

/* Windows enum issue with <= two sample rates work around */
#if ((MIN_FREQ == 8000) && (MAX_FREQ_FS == 11025)) \
    || (MIN_FREQ == 11025) && (MAX_FREQ_FS == 12000) \
    || (MIN_FREQ == 12000) && (MAX_FREQ_FS == 16000) \
    || (MIN_FREQ == 16000) && (MAX_FREQ_FS == 32000) \
    || (MIN_FREQ == 44100) && (MAX_FREQ_FS == 48000) \
    || (MIN_FREQ == 48000) && (MAX_FREQ_FS == 88200) \
    || (MIN_FREQ == 88200) && (MAX_FREQ_FS == 96000)
    CHARIFY_SR(MAX_FREQ_FS),
#endif
#if (MIN_FREQ == MAX_FREQ_FS)
    CHARIFY_SR(MAX_FREQ_FS),
    CHARIFY_SR(MAX_FREQ_FS),
#endif

#if(MIN_FREQ <= 8000) && (MAX_FREQ_FS >= 8000)
    0x40, 0x1F, 0x00,                     /* sampleFreq - 8000KHz */
#endif

#if(MIN_FREQ <= 11025) && (MAX_FREQ_FS >= 11025)
    0x11, 0x2B, 0x00,                     /* sampleFreq - 11025KHz */
#endif

#if(MIN_FREQ <= 12000) && (MAX_FREQ_FS >= 12000)
    0xE0, 0x2E, 0x00,                     /* sampleFreq - 12000KHz */
#endif

#if(MIN_FREQ <= 16000) && (MAX_FREQ_FS >= 16000)
    CHARIFY_SR(16000),                    /* sampleFreq - 32KHz */
#endif

#if(MIN_FREQ <= 32000) && (MAX_FREQ_FS >= 32000)
    CHARIFY_SR(32000),                    /* sampleFreq - 32KHz */
#endif

#if (MIN_FREQ <= 44100) && (MAX_FREQ_FS >= 44100)
    0x44, 0xAC, 0x00,                     /* sampleFreq - 44.1Khz */
#endif

#if (MIN_FREQ <= 48000) && (MAX_FREQ_FS >= 48000)
    0x80, 0xBB, 0x00,                     /* sampleFreq - 48KHz */
#endif

#if (MIN_FREQ <= 88200) && (MAX_FREQ_FS >= 88200)
    0x88, 0x58, 0x01,                     /* sampleFreq - 88.2KHz */
#endif

#if (MIN_FREQ <= 96000) && (MAX_FREQ_FS >= 96000)
    0x00, 0x77, 0x01,                     /* sampleFreq - 96KHz */
#endif

    /* Standard Endpoint Descriptor */
    0x09,
    0x05,                                 /* ENDPOINT */
    ENDPOINT_ADDRESS_IN_AUDIO,            /* EndpointAddress */
#if (XUA_SYNCMODE == XUA_SYNCMODE_ADAPT)
    ISO_EP_ATTRIBUTES_ADAPTIVE,
#elif (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    #if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP)
    ISO_EP_ATTRIBUTES_ASYNC,              /* Iso, async, data endpoint */
    #else
    ISO_EP_IMPL_ATTRIBUTES_ASYNC,         /* Feedback data endpoint */
    #endif
#elif (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
    ISO_EP_ATTRIBUTES_SYNC,               /* Iso, sync, data endpoint */
#else
#error "Unsupported XUA_SYNCMODE"
#endif
    FS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE&0xff,        /* 4  wMaxPacketSize (Typically 294 bytes)*/
    (FS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE&0xff00)>>8, /* 5  wMaxPacketSize */
    0x01,                                 /* bInterval */
    0x00,                                 /* bRefresh */
    0x00,                                 /* bSynchAddress */

    /* CS_Endpoint Descriptor */
    0x07,
    0x25,                                 /* CS_ENDPOINT */
    0x01,                                 /* Subtype - GENERAL */
    0x01,                                 /* Attributes. D[0]: sample freq ctrl. */
#if (XUA_SYNCMODE == XUA_SYNCMODE_ADAPT)
    0x02,                                 /* Lock Delay units PCM samples*/
    0x08, 0x00,                           /* No lock delay */
#else
    0x00,                                 /* Undefined */
    0x00, 0x00,                           /* Not used */
#endif
#endif // NUM_USB_CHAN_IN > 0

#if (XUA_DFU_EN == 1) && (FORCE_UAC1_DFU == 1)

    /* NOTE: By default we turn off DFU in UAC1.0 mode for a better user experiance in Windows */

    /* Standard DFU class Interface descriptor */
    0x09,                                /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                                 /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    (OUTPUT_INTERFACES_A1 + INPUT_INTERFACES_A1 + NUM_CONTROL_USB_INTERFACES + 1),  /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,                                 /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,                                 /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    0xFE,                                 /* 5 bInterfaceClass : DFU. (Application Specific) (field size 1 bytes) */
    0x01,                                 /* 6 bInterfaceSubclass : (field size 1 bytes) */
    0x01,                                 /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    offsetof(StringDescTable_t, dfuStr)/sizeof(char *), /* 8 iInterface */

    /* DFU 1.1 Run-Time DFU Functional Descriptor */
    0x09,                                 /* 0    Size */
    0x21,                                 /* 1    bDescriptorType : DFU FUNCTIONAL */
    0x07,                                 /* 2    bmAttributes */
    DFU_DETACH_TIME_OUT & 0xFF,           /* 3    wDetachTimeOut */
    (DFU_DETACH_TIME_OUT >> 8) & 0xFF,    /* 4    wDetachTimeOut */
    0x40,                                 /* 5    wTransferSize */
    0x00,                                 /* 6    wTransferSize */
    0x10,                                 /* 7    bcdDFUVersion */
    0x01,                                                  /* 7    bcdDFUVersion */
#endif

#ifdef USB_CONTROL_DESCS
    /* Control interface descriptor */
    0x09,                                                /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                                                /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    (OUTPUT_INTERFACES_A1 + INPUT_INTERFACES_A1 + 1),    /* 2 bInterfaceNumber */
    0x00,                                                /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,                                                /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    USB_CLASS_VENDOR_SPECIFIC,                           /* 5 bInterfaceClass : Vendor specific. (field size 1 bytes) */
    0xFF,                                                /* 6 bInterfaceSubclass : (field size 1 bytes) */
    0xFF,                                                /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    offsetof(StringDescTable_t, ctrlStr)/sizeof(char *), /* 8 iInterface */
#endif

#if XUA_OR_STATIC_HID_ENABLED
    #include "xua_hid_descriptors.h"
#endif

};
#endif // (AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS == 1)
#endif
#endif // _DEVICE_DESCRIPTORS_

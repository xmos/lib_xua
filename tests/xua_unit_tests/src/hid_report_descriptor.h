// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __hid_report_descriptor_h__
#define __hid_report_descriptor_h__

#include "xua_hid_report_descriptor.h"

#define MAX_VALID_BIT   ( 7 )
#define MAX_VALID_BYTE  ( 1 )

#define MIN_VALID_BIT   ( 0 )
#define MIN_VALID_BYTE  ( 0 )

#if 0
/* Existing static report descriptor kept for reference */
unsigned char hidReportDescriptor[] =
{
    0x05, 0x01,         /* Usage Page (Generic Desktop) */
    0x09, 0x06,         /* Usage (Keyboard) */
    0xa1, 0x01,         /* Collection (Application) */
    0x75, 0x01,         /* Report Size (1) */
    0x95, 0x04,         /* Report Count (4) */
    0x15, 0x00,         /* Logical Minimum (0) */
    0x25, 0x00,         /* Logical Maximum (0) */
    0x81, 0x01,         /* Input (Cnst, Ary, Abs, No Wrap, Lin, Pref, No Nul) */
    0x95, 0x01,         /* Report Count (1) */
    0x25, 0x01,         /* Logical Maximum (1) */
    0x05, 0x07,         /* Usage Page (Key Codes) */
    0x19, 0x17,         /* Usage Minimum (Keyboard t or T) */
    0x29, 0x17,         /* Usage Maximum (Keyboard t or T) */
    0x81, 0x02,         /* Input (Data, Var, Abs, No Wrap, Lin, Pref, No Nul) */
    0x05, 0x0C,         /* Usage Page (Consumer) */
    0x0a, 0x26, 0x02,   /* Usage (AC Stop) */
    0x81, 0x02,         /* Input (Data, Var, Abs, No Wrap, Lin, Pref, No Nul) */
    0x95, 0x02,         /* Report Count (2) */
    0x05, 0x07,         /* Usage Page (Key Codes) */
    0x19, 0x72,         /* Usage Minimum (Keyboard F23) */
    0x29, 0x73,         /* Usage Maximum (Keyboard F24) */
    0x81, 0x02,         /* Input (Data, Var, Abs, No Wrap, Lin, Pref, No Nul) */
    0xc0                /* End collection (Application) */
};
#endif

static const USB_HID_Short_Item_t hidCollectionApplication  = { .header = 0xA1, .data = { 0x01, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidCollectionEnd          = { .header = 0xC0, .data = { 0x00, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidCollectionLogical      = { .header = 0xA1, .data = { 0x02, 0x00 }, .location = 0x00 };

static const USB_HID_Short_Item_t hidInputConstArray        = { .header = 0x81, .data = { 0x01, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidInputDataVar           = { .header = 0x81, .data = { 0x02, 0x00 }, .location = 0x00 };

static const USB_HID_Short_Item_t hidLogicalMaximum0        = { .header = 0x25, .data = { 0x00, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidLogicalMaximum1        = { .header = 0x25, .data = { 0x01, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidLogicalMinimum0        = { .header = 0x15, .data = { 0x00, 0x00 }, .location = 0x00 };

static const USB_HID_Short_Item_t hidReportCount1           = { .header = 0x95, .data = { 0x01, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidReportCount6           = { .header = 0x95, .data = { 0x06, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidReportCount7           = { .header = 0x95, .data = { 0x07, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidReportSize1            = { .header = 0x75, .data = { 0x01, 0x00 }, .location = 0x00 };

static const USB_HID_Short_Item_t hidUsageConsumerControl   = { .header = 0x09, .data = { 0x01, 0x00 }, .location = 0x00 };

static const USB_HID_Short_Item_t hidUsagePageConsumer      = { .header = 0x05, .data = { 0x0C, 0x00 }, .location = 0x00 };

static USB_HID_Short_Item_t hidUsageByte0Bit0   = { .header = 0x09, .data = { 0xE2, 0x00 }, .location = 0x00 }; // Mute

static USB_HID_Short_Item_t hidUsageByte1Bit7   = { .header = 0x09, .data = { 0xEA, 0x00 }, .location = 0x71 }; // Vol-
static USB_HID_Short_Item_t hidUsageByte1Bit0   = { .header = 0x09, .data = { 0xE9, 0x00 }, .location = 0x01 }; // Vol+

static USB_HID_Short_Item_t* const hidConfigurableItems[] = {
    &hidUsageByte0Bit0,
    &hidUsageByte1Bit0,
    &hidUsageByte1Bit7
};

static const USB_HID_Short_Item_t* const hidReportDescriptorItems[] = {
    &hidUsagePageConsumer,
    &hidUsageConsumerControl,
    &hidCollectionApplication,
        &hidReportSize1,
        &hidLogicalMinimum0,
        &hidCollectionLogical,      // Byte 0
            &hidLogicalMaximum1,
            &hidReportCount1,
            &hidUsageByte0Bit0,
            &hidInputDataVar,
            &hidLogicalMaximum0,
            &hidReportCount7,
            &hidInputConstArray,
        &hidCollectionEnd,
        &hidCollectionLogical,      // Byte 1
            &hidLogicalMaximum1,
            &hidReportCount1,
            &hidUsageByte1Bit0,
            &hidInputDataVar,
            &hidLogicalMaximum0,
            &hidReportCount6,
            &hidInputConstArray,
            &hidLogicalMaximum1,
            &hidUsageByte1Bit7,
            &hidInputDataVar,
        &hidCollectionEnd,
    &hidCollectionEnd
};

#endif // __hid_report_descriptor_h__

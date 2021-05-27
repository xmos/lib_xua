// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __hid_report_descriptor_h__
#define __hid_report_descriptor_h__

#include "xua_hid_report_descriptor.h"

#define MAX_VALID_BIT   ( 7 )
#define MAX_VALID_BYTE  ( 1 )

#define MIN_VALID_BIT   ( 0 )
#define MIN_VALID_BYTE  ( 0 )

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

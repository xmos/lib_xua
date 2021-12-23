// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __hid_report_descriptor_h__
#define __hid_report_descriptor_h__

#include "xua_hid_report.h"

#define USB_HID_REPORT_ID_KEYBOARD  ( 0x01 )
#define USB_HID_REPORT_ID_CONSUMER  ( 0x02 )
#define USB_HID_REPORT_ID_TELEPHONY ( 0x03 )

/*
 * Define non-configurable items in the HID Report descriptor.
 * (These are short items as the location field isn't relevant for them)
 */
static const USB_HID_Short_Item_t hidCollectionApplication  = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_MAIN, HID_REPORT_ITEM_TAG_COLLECTION),
    .data = { 0x01, 0x00 } };
static const USB_HID_Short_Item_t hidCollectionEnd          = {
    .header = HID_REPORT_SET_HEADER(0, HID_REPORT_ITEM_TYPE_MAIN, HID_REPORT_ITEM_TAG_END_COLLECTION),
    .data = { 0x00, 0x00 } };


static const USB_HID_Short_Item_t hidInputConstArray        = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_MAIN, HID_REPORT_ITEM_TAG_INPUT),
    .data = { 0x01, 0x00 } };
static const USB_HID_Short_Item_t hidInputDataVar           = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_MAIN, HID_REPORT_ITEM_TAG_INPUT),
    .data = { 0x02, 0x00 } };

static const USB_HID_Short_Item_t hidLogicalMaximum0        = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_LOGICAL_MAXIMUM),
    .data = { 0x00, 0x00 } };
static const USB_HID_Short_Item_t hidLogicalMaximum1        = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_LOGICAL_MAXIMUM),
    .data = { 0x01, 0x00 } };
static const USB_HID_Short_Item_t hidLogicalMinimum0        = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_LOGICAL_MINIMUM),
    .data = { 0x00, 0x00 } };

static const USB_HID_Short_Item_t hidReportCount1           = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_REPORT_COUNT),
    .data = { 0x01, 0x00 } };
static const USB_HID_Short_Item_t hidReportCount4           = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_REPORT_COUNT),
    .data = { 0x04, 0x00 } };
static const USB_HID_Short_Item_t hidReportCount6           = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_REPORT_COUNT),
    .data = { 0x06, 0x00 } };
static const USB_HID_Short_Item_t hidReportCount7           = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_REPORT_COUNT),
    .data = { 0x07, 0x00 } };
static const USB_HID_Short_Item_t hidReportSize1            = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_REPORT_SIZE),
    .data = { 0x01, 0x00 } };

static const USB_HID_Short_Item_t hidUsagePageGenericDesktop  = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_USAGE_PAGE),
    .data = { USB_HID_USAGE_PAGE_ID_GENERIC_DESKTOP, 0x00 }};
static const USB_HID_Short_Item_t hidUsagePageConsumerControl = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_USAGE_PAGE),
    .data = { USB_HID_USAGE_PAGE_ID_CONSUMER, 0x00 }};
static const USB_HID_Short_Item_t hidUsagePageTelephony       = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_USAGE_PAGE),
    .data = { USB_HID_USAGE_PAGE_ID_TELEPHONY_DEVICE, 0x00 }};

static const USB_HID_Short_Item_t hidUsageKeyboard            = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .data = { 0x06, 0x00 }};
static const USB_HID_Short_Item_t hidUsageConsumerControl     = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .data = { 0x01, 0x00 }};
static const USB_HID_Short_Item_t hidUsageTelephonyHeadset    = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .data = { 0x05, 0x00 }};

/*
 * Define the HID Report Descriptor Item, Usage Page, Report ID and length for each HID Report
 * For internal purposes, a report element with ID of 0 must be included if report IDs are not being used.
 */
static const USB_HID_Short_Item_t hidReportId1  = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_REPORT_ID),
    .data = { USB_HID_REPORT_ID_KEYBOARD,  0x00 } };
static const USB_HID_Short_Item_t hidReportId2  = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_REPORT_ID),
    .data = { USB_HID_REPORT_ID_CONSUMER,  0x00 } };
static const USB_HID_Short_Item_t hidReportId3  = {
    .header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_REPORT_ID),
    .data = { USB_HID_REPORT_ID_TELEPHONY, 0x00 } };

static const USB_HID_Report_Element_t hidReportKeyboard     = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_USAGE_PAGE),
    .item.data = { USB_HID_USAGE_PAGE_ID_KEYBOARD,  0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_KEYBOARD, 1, 0, 0 )
};

static const USB_HID_Report_Element_t hidReportConsumer     = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_USAGE_PAGE),
    .item.data = { USB_HID_USAGE_PAGE_ID_CONSUMER, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 2, 0, 0 )
};

static const USB_HID_Report_Element_t hidReportTelephony    = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_GLOBAL, HID_REPORT_ITEM_TAG_USAGE_PAGE),
    .item.data = { USB_HID_USAGE_PAGE_ID_TELEPHONY_DEVICE, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_TELEPHONY, 1, 0, 0 )
};

/*
 * Define configurable elements in the HID Report descriptor.
 */
static USB_HID_Report_Element_t hidUsageReport1Byte0Bit0    = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0x17, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_KEYBOARD, 0, 0, 0 )
}; // 't'

static USB_HID_Report_Element_t hidUsageReport1Byte0Bit2    = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0x72, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_KEYBOARD, 0, 0, 2 )
}; // F23

static USB_HID_Report_Element_t hidUsageReport1Byte0Bit3    = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0x73, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_KEYBOARD, 0, 0, 3 )
}; // F24

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit0    = {
    .item.header = HID_REPORT_SET_HEADER(2, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0x26, 0x02 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 0 )
}; // AC Stop

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit1    = {
    .item.header = HID_REPORT_SET_HEADER(2, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0x21, 0x02 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 1 )
}; // AC Search

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit2    = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0xE2, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 2 )
}; // Mute

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit4    = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0xCF, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 4 )
}; // Voice Command

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit6    = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0xE9, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 6 )
}; // Vol+

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit7    = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0xEA, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 7 )
}; // Vol-

static USB_HID_Report_Element_t hidUsageReport2Byte1Bit7    = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0xE5, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 1, 7 )
}; // Bass boost

static USB_HID_Report_Element_t hidUsageReport3Byte0Bit0    = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0x20, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_TELEPHONY, 0, 0, 0 )
}; // Hook Switch

static USB_HID_Report_Element_t hidUsageReport3Byte0Bit1    = {
    .item.header = HID_REPORT_SET_HEADER(1, HID_REPORT_ITEM_TYPE_LOCAL, HID_REPORT_ITEM_TAG_USAGE),
    .item.data = { 0x2F, 0x00 },
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_TELEPHONY, 0, 0, 1 )
}; // Phone Mute

/*
 * List the configurable elements in the HID Report.
 */
static USB_HID_Report_Element_t* const hidConfigurableElements[] = {
    &hidUsageReport1Byte0Bit0,
    &hidUsageReport1Byte0Bit2,
    &hidUsageReport1Byte0Bit3,

    &hidUsageReport2Byte0Bit0,
    &hidUsageReport2Byte0Bit1,
    &hidUsageReport2Byte0Bit2,
    &hidUsageReport2Byte0Bit4,
    &hidUsageReport2Byte0Bit6,
    &hidUsageReport2Byte0Bit7,
    &hidUsageReport2Byte1Bit7,

    &hidUsageReport3Byte0Bit0,
    &hidUsageReport3Byte0Bit1
};

/*
 * List HID Reports, one per Report ID. This should be a usage page item with the locator filled out with ID and size
 * If not using report IDs - still have one with report ID 0
 */
static const USB_HID_Report_Element_t* const hidReports[] = {
    &hidReportKeyboard,
    &hidReportConsumer,
    &hidReportTelephony
};

/*
 * List all items in the HID Report descriptor.
 */
static const USB_HID_Short_Item_t* const hidReportDescriptorItems[] = {
    &hidUsagePageGenericDesktop,
    &hidUsageKeyboard,
    &hidReportSize1,
    &hidLogicalMinimum0,
    &hidCollectionApplication,      // Report 1
        &hidReportId1,
        &(hidReportKeyboard.item),
        &hidLogicalMaximum1,
        &hidReportCount1,
        &(hidUsageReport1Byte0Bit0.item),
        &hidInputDataVar,
        &hidLogicalMaximum0,
        &hidInputConstArray,
        &hidLogicalMaximum1,
        &(hidUsageReport1Byte0Bit2.item),
        &hidInputDataVar,
        &(hidUsageReport1Byte0Bit3.item),
        &hidInputDataVar,
        &hidLogicalMaximum0,
        &hidReportCount4,
        &hidInputConstArray,
    &hidCollectionEnd,
    &hidUsagePageConsumerControl,
    &hidUsageConsumerControl,
    &hidCollectionApplication,      // Report 2
        &hidReportId2,
        &(hidReportConsumer.item),
        &hidLogicalMaximum1,
        &hidReportCount1,
        &(hidUsageReport2Byte0Bit0.item),
        &hidInputDataVar,
        &(hidUsageReport2Byte0Bit1.item),
        &hidInputDataVar,
        &(hidUsageReport2Byte0Bit2.item),
        &hidInputDataVar,
        &hidLogicalMaximum0,
        &hidInputConstArray,
        &hidLogicalMaximum1,
        &(hidUsageReport2Byte0Bit4.item),
        &hidInputDataVar,
        &hidLogicalMaximum0,
        &hidInputConstArray,
        &hidLogicalMaximum1,
        &(hidUsageReport2Byte0Bit6.item),
        &hidInputDataVar,
        &(hidUsageReport2Byte0Bit7.item),
        &hidInputDataVar,
        &hidLogicalMaximum0,
        &hidReportCount7,
        &hidInputConstArray,
        &hidLogicalMaximum1,
        &hidReportCount1,
        &(hidUsageReport2Byte1Bit7.item),
        &hidInputDataVar,
    &hidCollectionEnd,
    &hidUsagePageTelephony,
    &hidUsageTelephonyHeadset,
    &hidCollectionApplication,      // Report 3
        &hidReportId3,
        &(hidReportTelephony.item),
        &(hidUsageReport3Byte0Bit0.item),
        &hidInputDataVar,
        &(hidUsageReport3Byte0Bit1.item),
        &hidInputDataVar,
        &hidLogicalMaximum0,
        &hidReportCount6,
        &hidInputConstArray,
    &hidCollectionEnd,
};

/*
 * Define the number of HID Reports
 * Due to XC not supporting designated initializers, this constant has a hard-coded value.
 * It must equal ( sizeof hidReports / sizeof ( USB_HID_Report_Element_t* ))
 */
#define HID_REPORT_COUNT ( 3 )

#endif // __hid_report_descriptor_h__

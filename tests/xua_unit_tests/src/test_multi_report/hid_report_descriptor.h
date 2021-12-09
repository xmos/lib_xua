// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __hid_report_descriptor_h__
#define __hid_report_descriptor_h__

#include "xua_hid_report_descriptor.h"

#define REPORT1_MAX_VALID_BIT   ( 3 )
#define REPORT1_MAX_VALID_BYTE  ( 0 )
#define REPORT1_MIN_VALID_BIT   ( 0 )
#define REPORT1_MIN_VALID_BYTE  ( 0 )

#define REPORT2_MAX_VALID_BIT   ( 7 )
#define REPORT2_MAX_VALID_BYTE  ( 1 )
#define REPORT2_MIN_VALID_BIT   ( 0 )
#define REPORT2_MIN_VALID_BYTE  ( 0 )

#define REPORT3_MAX_VALID_BIT   ( 1 )
#define REPORT3_MAX_VALID_BYTE  ( 0 )
#define REPORT3_MIN_VALID_BIT   ( 0 )
#define REPORT3_MIN_VALID_BYTE  ( 0 )

#define USB_HID_REPORT_ID_KEYBOARD  ( 0x01 )
#define USB_HID_REPORT_ID_CONSUMER  ( 0x02 )
#define USB_HID_REPORT_ID_TELEPHONY ( 0x03 )

#define USB_HID_USAGE_PAGE_ID_CONSUMER  ( 0x0C )
#define USB_HID_USAGE_PAGE_ID_KEYBOARD  ( 0x07 )
#define USB_HID_USAGE_PAGE_ID_TELEPHONY ( 0x0B )

/*
 * Define non-configurable items in the HID Report descriptor.
 * (These are short items as the location field isn't relevant for them)
 */
static const USB_HID_Short_Item_t hidCollectionApplication  = { .header = 0xA1, .data = { 0x01, 0x00 }};
static const USB_HID_Short_Item_t hidCollectionEnd          = { .header = 0xC0, .data = { 0x00, 0x00 }};
static const USB_HID_Short_Item_t hidCollectionLogical      = { .header = 0xA1, .data = { 0x02, 0x00 }};

static const USB_HID_Short_Item_t hidInputConstArray        = { .header = 0x81, .data = { 0x01, 0x00 }};
static const USB_HID_Short_Item_t hidInputDataVar           = { .header = 0x81, .data = { 0x02, 0x00 }};

static const USB_HID_Short_Item_t hidLogicalMaximum0        = { .header = 0x25, .data = { 0x00, 0x00 }};
static const USB_HID_Short_Item_t hidLogicalMaximum1        = { .header = 0x25, .data = { 0x01, 0x00 }};
static const USB_HID_Short_Item_t hidLogicalMinimum0        = { .header = 0x15, .data = { 0x00, 0x00 }};

static const USB_HID_Short_Item_t hidReportCount1           = { .header = 0x95, .data = { 0x01, 0x00 }};
static const USB_HID_Short_Item_t hidReportCount4           = { .header = 0x95, .data = { 0x04, 0x00 }};
static const USB_HID_Short_Item_t hidReportCount6           = { .header = 0x95, .data = { 0x06, 0x00 }};
static const USB_HID_Short_Item_t hidReportSize1            = { .header = 0x75, .data = { 0x01, 0x00 }};

static const USB_HID_Short_Item_t hidUsageConsumerControl   = { .header = 0x09, .data = { 0x01, 0x00 }};

/*
 * Define the HID Report Descriptor Item, Usage Page, Report ID and length for each HID Report
 * For internal purposes, a report element with ID of 0 must be included if report IDs are not being used.
 */
static const USB_HID_Short_Item_t hidReportId1  = { .header = 0x85, .data = { USB_HID_REPORT_ID_KEYBOARD,  0x00 }};
static const USB_HID_Short_Item_t hidReportId2  = { .header = 0x85, .data = { USB_HID_REPORT_ID_CONSUMER,  0x00 }};
static const USB_HID_Short_Item_t hidReportId3  = { .header = 0x85, .data = { USB_HID_REPORT_ID_TELEPHONY, 0x00 }};

static const USB_HID_Report_Element_t hidReportKeyboard     = {
    .item = { .header = 0x05, .data = { USB_HID_USAGE_PAGE_ID_KEYBOARD,  0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_KEYBOARD, 1, 0, 0 )
};

static const USB_HID_Report_Element_t hidReportConsumer     = {
    .item = { .header = 0x05, .data = { USB_HID_USAGE_PAGE_ID_CONSUMER, 0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 2, 0, 0 )
};

static const USB_HID_Report_Element_t hidReportTelephony    = {
    .item = { .header = 0x05, .data = { USB_HID_USAGE_PAGE_ID_TELEPHONY, 0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_TELEPHONY, 1, 0, 0 )
};

static USB_HID_Report_Element_t hidUsageReport1Byte0Bit0    = {
    .item = { .header = 0x09, .data = { 0x17, 0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_KEYBOARD, 0, 0, 0 )
}; // 't'

static USB_HID_Report_Element_t hidUsageReport1Byte0Bit2    = {
    .item = { .header = 0x09, .data = { 0x72, 0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_KEYBOARD, 0, 0, 2 )
}; // F23

static USB_HID_Report_Element_t hidUsageReport1Byte0Bit3    = {
    .item = { .header = 0x09, .data = { 0x73, 0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_KEYBOARD, 0, 0, 3 )
}; // F24

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit0    = {
    .item = { .header = 0x0A, .data = { 0x26, 0x02 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 0 )
}; // AC Stop

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit1    = {
    .item = { .header = 0x0A, .data = { 0x21, 0x02 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 1 )
}; // AC Search

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit2    = {
    .item = { .header = 0x09, .data = { 0xE2, 0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 2 )
}; // Mute

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit4    = {
    .item = { .header = 0x09, .data = { 0xCF, 0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 4 )
}; // Voice Command

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit6    = {
    .item = { .header = 0x09, .data = { 0xE9, 0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 6 )
}; // Vol+

static USB_HID_Report_Element_t hidUsageReport2Byte0Bit7    = {
    .item = { .header = 0x09, .data = { 0xEA, 0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 0, 7 )
}; // Vol-

static USB_HID_Report_Element_t hidUsageReport2Byte1Bit7    = {
    .item = { .header = 0x09, .data = { 0xE5, 0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_CONSUMER, 0, 1, 7 )
}; // Bass boost

static USB_HID_Report_Element_t hidUsageReport3Byte0Bit0    = {
    .item = { .header = 0x09, .data = { 0x20, 0x00 }},
    .location = HID_REPORT_SET_LOC( USB_HID_REPORT_ID_TELEPHONY, 0, 0, 0 )
}; // Hook Switch

static USB_HID_Report_Element_t hidUsageReport3Byte0Bit1    = {
    .item = { .header = 0x09, .data = { 0x2F, 0x00 }},
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
 * List HID Reports, one per Report ID. This should be a usage page item with the relevant 
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
    &(hidReportConsumer.item),
    &hidUsageConsumerControl,
    &hidCollectionApplication,
        &hidReportSize1,
        &hidLogicalMinimum0,
        &hidCollectionLogical,      // Report 1
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
        &hidCollectionLogical,      // Report 2
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
            &hidReportCount6,
            &hidInputConstArray,
            &hidLogicalMaximum1,
            &(hidUsageReport2Byte1Bit7.item),
            &hidInputDataVar,
        &hidCollectionEnd,
        &hidCollectionLogical,      // Report 3
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
    &hidCollectionEnd
};

/*
 * Define the length of the HID Report.
 * This value must match the number of Report bytes defined by hidReportDescriptorItems.
 */
#define HID_REPORT_LENGTH   ( 3 )

/*
 * Define the number of HID Reports
 * Due to XC not supporting designated initializers, this constant has a hard-coded value.
 * It must equal ( sizeof hidReports / sizeof ( USB_HID_Report_Element_t* ))
 */
#define HID_REPORT_COUNT ( 3 )

#endif // __hid_report_descriptor_h__

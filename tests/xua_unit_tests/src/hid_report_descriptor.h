// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __hid_report_descriptor_h__
#define __hid_report_descriptor_h__

#include "xua_hid_report_descriptor.h"

#define MAX_VALID_BIT   ( 7 )
#define MAX_VALID_BYTE  ( 1 )

#define MIN_VALID_BIT   ( 0 )
#define MIN_VALID_BYTE  ( 0 )

#define USB_HID_USAGE_PAGE_ID_CONSUMER  ( 0x0C )

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
static const USB_HID_Short_Item_t hidReportCount6           = { .header = 0x95, .data = { 0x06, 0x00 }};
static const USB_HID_Short_Item_t hidReportCount7           = { .header = 0x95, .data = { 0x07, 0x00 }};
static const USB_HID_Short_Item_t hidReportSize1            = { .header = 0x75, .data = { 0x01, 0x00 }};

static const USB_HID_Short_Item_t hidUsageConsumerControl   = { .header = 0x09, .data = { 0x01, 0x00 }};

/*
 * Define the HID Report Descriptor Item, Usage Page, Report ID and length for each HID Report
 * For internal purposes, a report element with ID of 0 must be included if report IDs are not being used.
 */
static const USB_HID_Report_Element_t hidReportPageConsumer = {
    .item = { .header = 0x05, .data = { USB_HID_USAGE_PAGE_ID_CONSUMER, 0x00 }},
    .location = HID_REPORT_SET_LOC( 0, 2, 0, 0 )
};

/*
 * Define configurable items in the HID Report descriptor.
 */
static USB_HID_Report_Element_t hidUsageByte0Bit0 = {
    .item = { .header = 0x09, .data = { 0xE2, 0x00 }},
    .location = HID_REPORT_SET_LOC(0, 0, 0, 0)
}; // Mute

static USB_HID_Report_Element_t hidUsageByte1Bit7 = {
    .item = { .header = 0x09, .data = { 0xEA, 0x00 }},
    .location = HID_REPORT_SET_LOC(0, 0, 1, 7)
}; // Vol-
static USB_HID_Report_Element_t hidUsageByte1Bit0  = {
    .item = { .header = 0x09, .data = { 0xE9, 0x00 }},
    .location = HID_REPORT_SET_LOC(0, 0, 1, 0)
}; // Vol+

/*
 * List the configurable elements in the HID Report descriptor.
 */
static USB_HID_Report_Element_t* const hidConfigurableElements[] = {
    &hidUsageByte0Bit0,
    &hidUsageByte1Bit0,
    &hidUsageByte1Bit7
};

/*
 * List HID Reports, one per Report ID. This should be a usage page item with the relevant 
 * If not using report IDs - still have one with report ID 0
 */
static const USB_HID_Report_Element_t* const hidReports[] = {
    &hidReportPageConsumer
};

/*
 * List all items in the HID Report descriptor.
 */
static const USB_HID_Short_Item_t* const hidReportDescriptorItems[] = {
    &(hidReportPageConsumer.item),
    &hidUsageConsumerControl,
    &hidCollectionApplication,
        &hidReportSize1,
        &hidLogicalMinimum0,
        &hidCollectionLogical,      // Byte 0
            &hidLogicalMaximum1,
            &hidReportCount1,
            &(hidUsageByte0Bit0.item),
            &hidInputDataVar,
            &hidLogicalMaximum0,
            &hidReportCount7,
            &hidInputConstArray,
        &hidCollectionEnd,
        &hidCollectionLogical,      // Byte 1
            &hidLogicalMaximum1,
            &hidReportCount1,
            &(hidUsageByte1Bit0.item),
            &hidInputDataVar,
            &hidLogicalMaximum0,
            &hidReportCount6,
            &hidInputConstArray,
            &hidLogicalMaximum1,
            &(hidUsageByte1Bit7.item),
            &hidInputDataVar,
        &hidCollectionEnd,
    &hidCollectionEnd
};

/*
 * Define the length of the HID Report.
 * This value must match the number of Report bytes defined by hidReportDescriptorItems.
 */
#define HID_REPORT_LENGTH   ( 2 )

/*
 * Define the number of HID Reports
 * Due to XC not supporting designated initializers, this constant has a hard-coded value.
 * It must equal ( sizeof hidReports / sizeof ( USB_HID_Report_Element_t* ))
 */
#define HID_REPORT_COUNT ( 1 )

#endif // __hid_report_descriptor_h__

// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) Report descriptor constants
 *
 * This file defines a collection of constants from the USB HID documents.
 * This includes constants from:
 *  - Device Class Definition for Human Interface Devices, version 1.11
 *  - HID Usage Tables for Universal Serial Bus, version 1.22
 *
 * This file is incomplete, but can be expanded with new constants as necessary.
 */

#ifndef _XUA_HID_REPORT_DESCRIPTOR_CONSTANTS_
#define _XUA_HID_REPORT_DESCRIPTOR_CONSTANTS_


// Constants from the USB Device Class Definition for HID for type
#define HID_REPORT_ITEM_TYPE_MAIN              ( 0x00 )
#define HID_REPORT_ITEM_TYPE_GLOBAL            ( 0x01 )
#define HID_REPORT_ITEM_TYPE_LOCAL             ( 0x02 )
#define HID_REPORT_ITEM_TYPE_RESERVED          ( 0x03 )

// Constants from the USB Device Class Definition for HID for tag
// Main items
#define HID_REPORT_ITEM_TAG_INPUT              ( 0x08 )
#define HID_REPORT_ITEM_TAG_OUTPUT             ( 0x09 )
#define HID_REPORT_ITEM_TAG_FEATURE            ( 0x0B )
#define HID_REPORT_ITEM_TAG_COLLECTION         ( 0x0A )
#define HID_REPORT_ITEM_TAG_END_COLLECTION     ( 0x0C )

// Global items
#define HID_REPORT_ITEM_TAG_USAGE_PAGE         ( 0x00 )
#define HID_REPORT_ITEM_TAG_LOGICAL_MINIMUM    ( 0x01 )
#define HID_REPORT_ITEM_TAG_LOGICAL_MAXIMUM    ( 0x02 )
#define HID_REPORT_ITEM_TAG_PHYSICAL_MINIMUM   ( 0x03 )
#define HID_REPORT_ITEM_TAG_PHYSICAL_MAXIMUM   ( 0x04 )
#define HID_REPORT_ITEM_TAG_UNIT_EXPONENT      ( 0x05 )
#define HID_REPORT_ITEM_TAG_UNIT               ( 0x06 )
#define HID_REPORT_ITEM_TAG_REPORT_SIZE        ( 0x07 )
#define HID_REPORT_ITEM_TAG_REPORT_ID          ( 0x08 )
#define HID_REPORT_ITEM_TAG_REPORT_COUNT       ( 0x09 )
#define HID_REPORT_ITEM_TAG_PUSH               ( 0x0A )
#define HID_REPORT_ITEM_TAG_POP                ( 0x0B )

// Local items
#define HID_REPORT_ITEM_TAG_USAGE              ( 0x00 )
#define HID_REPORT_ITEM_TAG_USAGE_MINIMUM      ( 0x01 )
#define HID_REPORT_ITEM_TAG_USAGE_MAXIMUM      ( 0x02 )
#define HID_REPORT_ITEM_TAG_DESIGNATOR_INDEX   ( 0x03 )
#define HID_REPORT_ITEM_TAG_DESIGNATOR_MINIMUM ( 0x04 )
#define HID_REPORT_ITEM_TAG_DESIGNATOR_MAXIMUM ( 0x05 )
#define HID_REPORT_ITEM_TAG_STRING_INDEX       ( 0x07 )
#define HID_REPORT_ITEM_TAG_STRING_MINIMUM     ( 0x08 )
#define HID_REPORT_ITEM_TAG_STRING_MAXIMUM     ( 0x09 )
#define HID_REPORT_ITEM_TAG_DELIMITER          ( 0x0A )

// Constants from HID Usage Tables
// Usage page IDs (incomplete)
#define USB_HID_USAGE_PAGE_ID_GENERIC_DESKTOP  ( 0x01 )
#define USB_HID_USAGE_PAGE_ID_KEYBOARD         ( 0x07 )
#define USB_HID_USAGE_PAGE_ID_TELEPHONY_DEVICE ( 0x0B )
#define USB_HID_USAGE_PAGE_ID_CONSUMER         ( 0x0C )

#endif // _XUA_HID_REPORT_DESCRIPTOR_CONSTANTS_

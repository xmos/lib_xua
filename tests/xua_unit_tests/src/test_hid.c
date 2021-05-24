// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>

#include "xua_unit_tests.h"
#include "xua_hid_report_descriptor.h"

static unsigned construct_usage_header( unsigned size )
{
    unsigned header = 0x00;

    header |= ( HID_REPORT_ITEM_USAGE_TAG  << HID_REPORT_ITEM_HDR_TAG_SHIFT  ) & HID_REPORT_ITEM_HDR_TAG_MASK;
    header |= ( HID_REPORT_ITEM_USAGE_TYPE << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK;

    header |= ( size << HID_REPORT_ITEM_HDR_SIZE_SHIFT ) & HID_REPORT_ITEM_HDR_SIZE_MASK;

    return header;
}

void test_uninitialised_hidGetReportDescriptor( void )
{
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );
}

void test_initialised_hidGetReportDescriptor( void )
{
    hidInitReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );
}

void test_bad_bit_hidSetReportItem( void )
{
    const unsigned bit = 8;
    const unsigned byte = 0;
    const unsigned char header = construct_usage_header( 0 );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

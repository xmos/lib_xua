// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>
#include <stdio.h>

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

// Basic report descriptor tests
void test_unprepared_hidGetReportDescriptor( void )
{
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );
}

void test_prepared_hidGetReportDescriptor( void )
{
    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );
}

// Configurable and non-configurable item tests
void test_configurable_item_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char data[ 1 ] = { 0x2C };   // Spacebar keycode
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));

    unsigned retVal = hidSetReportItem( byte, bit, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_nonconfigurable_item_hidSetReportItem( void )
{
    const unsigned bit = 7; // Reserved bit
    const unsigned byte = 0;
    const unsigned char data[ 1 ] = { 0x2C };   // Spacebar keycode
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));

    unsigned retVal = hidSetReportItem( byte, bit, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

// Bit range tests
void test_max_bit_hidSetReportItem( void )
{
    const unsigned bit = 7;
    const unsigned byte = 1;    // Only byte 1 has bit 7 not reserved
    const unsigned char header = construct_usage_header( 0 );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_min_bit_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char header = construct_usage_header( 0 );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_overflow_bit_hidSetReportItem( void )
{
    const unsigned bit = 8;
    const unsigned byte = 0;
    const unsigned char header = construct_usage_header( 0 );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

void test_underflow_bit_hidSetReportItem( void )
{
    const int bit = -1;
    const unsigned byte = 0;
    const unsigned char header = construct_usage_header( 0 );

    unsigned retVal = hidSetReportItem( byte, ( unsigned ) bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

// Byte range tests
void test_max_byte_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 2;
    const unsigned char header = construct_usage_header( 0 );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_min_byte_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char header = construct_usage_header( 0 );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_overflow_byte_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 4;
    const unsigned char header = construct_usage_header( 0 );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

void test_underflow_byte_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const int byte = -1;
    const unsigned char header = construct_usage_header( 0 );

    unsigned retVal = hidSetReportItem( ( unsigned ) byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

// Size range tests
void test_max_size_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0x00 };
    const unsigned char header = construct_usage_header( HID_REPORT_ITEM_MAX_SIZE );

    unsigned retVal = hidSetReportItem( byte, bit, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_min_size_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char header = construct_usage_header( 0x00 );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_unsupported_size_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char header = construct_usage_header( 0x03 );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

// Header tag and type tests
void test_bad_tag_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char good_header = construct_usage_header( 0x00 );

    for( unsigned tag = 0x01; tag <= 0x0F; ++tag ) {
        unsigned char bad_header = good_header | (( 0x0F << HID_REPORT_ITEM_HDR_TAG_SHIFT ) & HID_REPORT_ITEM_HDR_TAG_MASK );
        unsigned retVal = hidSetReportItem( byte, bit, bad_header, NULL );
        TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
    }
}

void test_local_type_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char header = ( construct_usage_header( 0x00 ) &
                                 ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
                                 (( 0x02 << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_global_type_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char header = ( construct_usage_header( 0x00 ) &
                                 ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
                                 (( 0x01 << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

void test_main_type_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char header = ( construct_usage_header( 0x00 ) &
                                 ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
                                 (( 0x00 << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

void test_reserved_type_hidSetReportItem( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char header = ( construct_usage_header( 0x00 ) &
                                 ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
                                 (( 0x03 << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );

    unsigned retVal = hidSetReportItem( byte, bit, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

// Combined function tests
void test_initial_modification_without_subsequent_preparation( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char data[ 1 ] = { 0x2C };   // Spacebar keycode
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));

    unsigned retVal = hidSetReportItem( byte, bit, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );
}

void test_initial_modification_with_subsequent_preparation( void )
{
    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char data[ 1 ] = { 0x2C };   // Spacebar keycode
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));

    unsigned retVal = hidSetReportItem( byte, bit, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );
}

void test_modification_without_subsequent_preparation( void )
{
    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );

    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char data[ 1 ] = { 0x2C };   // Spacebar keycode
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));

    unsigned retVal = hidSetReportItem( byte, bit, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );
}

void test_modification_with_subsequent_preparation( void )
{
    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );

    const unsigned bit = 0;
    const unsigned byte = 0;
    const unsigned char data[ 1 ] = { 0x2C };   // Spacebar keycode
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));

    unsigned retVal = hidSetReportItem( byte, bit, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    hidPrepareReportDescriptor();
    reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );
}

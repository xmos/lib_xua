// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>
#include <stdio.h>

#include "xua_unit_tests.h"
#include "xua_hid_report.h"

// Test constants related to the report descriptor defined in hid_report_descriptor.h
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

#define HID_REPORT_LENGTH  ( 3 )
#define HID_REPORT_COUNT ( 3 )
#define HID_REPORTID_LIMIT ( 4 )

// Constants from USB HID Usage Tables
#define KEYBOARD_PAGE           ( 0x07 )
#define CONSUMER_PAGE           ( 0x0C )
#define TELEPHONY_DEVICE_PAGE   ( 0x0B )
#define LOUDNESS_CONTROL        ( 0xE7 )
#define PHONE_KEY_9             ( 0xB9 )
#define KEYBOARD_X              ( 0x1B )
#define PHONE_HOST_HOLD         ( 0x010A )

static unsigned construct_usage_header( unsigned size )
{
    unsigned header = 0x00;

    header |= ( HID_REPORT_ITEM_USAGE_TAG  << HID_REPORT_ITEM_HDR_TAG_SHIFT  ) & HID_REPORT_ITEM_HDR_TAG_MASK;
    header |= ( HID_REPORT_ITEM_USAGE_TYPE << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK;

    header |= ( size << HID_REPORT_ITEM_HDR_SIZE_SHIFT ) & HID_REPORT_ITEM_HDR_SIZE_MASK;

    return header;
}

void setUp( void )
{
    hidReportInit();
    hidResetReportDescriptor();
}

void test_validate_report( void ) {
    unsigned retVal = hidReportValidate();
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_reportid_in_use( void ) {
    unsigned reportIdInUse = hidIsReportIdInUse();
    TEST_ASSERT_EQUAL_UINT( 1, reportIdInUse );
}

void test_get_next_valid_report_id( void ) {
    unsigned reportId = 0U;

    reportId = hidGetNextValidReportId(reportId);
    TEST_ASSERT_EQUAL_UINT( 1, reportId );

    reportId = hidGetNextValidReportId(reportId);
    TEST_ASSERT_EQUAL_UINT( 2, reportId );

    reportId = hidGetNextValidReportId(reportId);
    TEST_ASSERT_EQUAL_UINT( 3, reportId );

    reportId = hidGetNextValidReportId(reportId);
    TEST_ASSERT_EQUAL_UINT( 1, reportId );
}

void test_is_report_id_valid( void ) {
    unsigned isValid = 0;

    unsigned reportId = 0;
    isValid = hidIsReportIdValid( reportId );
    TEST_ASSERT_EQUAL_UINT( 0, isValid );

    reportId = 1;
    isValid = hidIsReportIdValid( reportId );
    TEST_ASSERT_EQUAL_UINT( 1, isValid );

    reportId = 2;
    isValid = hidIsReportIdValid( reportId );
    TEST_ASSERT_EQUAL_UINT( 1, isValid );

    reportId = 3;
    isValid = hidIsReportIdValid( reportId );
    TEST_ASSERT_EQUAL_UINT( 1, isValid );

    reportId = 4;
    isValid = hidIsReportIdValid( reportId );
    TEST_ASSERT_EQUAL_UINT( 0, isValid );
}

// Basic report descriptor tests
void test_unprepared_hidGetReportDescriptor( void )
{
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );

    for (unsigned reportId = 1; reportId <= HID_REPORT_COUNT; reportId++)
    {
        unsigned reportLength = hidGetReportLength( reportId );
        TEST_ASSERT_EQUAL_UINT( 0, reportLength );
    }
}

void test_prepared_hidGetReportDescriptor( void )
{
    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );

    unsigned reportId = 1;
    unsigned reportLength = hidGetReportLength( reportId );
    TEST_ASSERT_EQUAL_UINT( 1, reportLength );

    reportId = 2;
    reportLength = hidGetReportLength( reportId );
    TEST_ASSERT_EQUAL_UINT( 2, reportLength );

    reportId = 3;
    reportLength = hidGetReportLength( reportId );
    TEST_ASSERT_EQUAL_UINT( 1, reportLength );
}

void test_reset_unprepared_hidGetReportDescriptor( void )
{
    hidPrepareReportDescriptor();
    hidResetReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );
}

void test_reset_prepared_hidGetReportDescriptor( void )
{
    hidPrepareReportDescriptor();
    hidResetReportDescriptor();
    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );
}

void test_report_id_limit( void )
{
    unsigned reportIdLimit = hidGetReportIdLimit();
    TEST_ASSERT_EQUAL_UINT( HID_REPORTID_LIMIT, reportIdLimit );
}

// Basic item tests
void test_max_loc_hidGetReportItem( void )
{
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ];
    unsigned char header;
    unsigned char page;

    unsigned reportId = 1;
    unsigned bit = REPORT1_MAX_VALID_BIT;
    unsigned byte = REPORT1_MAX_VALID_BYTE;
    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
    TEST_ASSERT_EQUAL_UINT( KEYBOARD_PAGE, page );
    TEST_ASSERT_EQUAL_UINT( 0x09, header );
    TEST_ASSERT_EQUAL_UINT( 0x73, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0x00, data[ 1 ]);

    reportId = 2;
    bit = REPORT2_MAX_VALID_BIT;
    byte = REPORT2_MAX_VALID_BYTE;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
    TEST_ASSERT_EQUAL_UINT( CONSUMER_PAGE, page );
    TEST_ASSERT_EQUAL_UINT( 0x09, header );
    TEST_ASSERT_EQUAL_UINT( 0xE5, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0x00, data[ 1 ]);

    reportId = 3;
    bit = REPORT3_MAX_VALID_BIT;
    byte = REPORT3_MAX_VALID_BYTE;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
    TEST_ASSERT_EQUAL_UINT( TELEPHONY_DEVICE_PAGE, page );
    TEST_ASSERT_EQUAL_UINT( 0x09, header );
    TEST_ASSERT_EQUAL_UINT( 0x2F, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0x00, data[ 1 ]);
}


void test_min_loc_hidGetReportItem( void )
{
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ];
    unsigned char header;
    unsigned char page;

    unsigned reportId = 1;
    unsigned bit = REPORT1_MIN_VALID_BIT;
    unsigned byte = REPORT1_MIN_VALID_BYTE;
    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
    TEST_ASSERT_EQUAL_UINT( KEYBOARD_PAGE, page );
    TEST_ASSERT_EQUAL_UINT( 0x09, header );
    TEST_ASSERT_EQUAL_UINT( 0x17, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0x00, data[ 1 ]);

    reportId = 2;
    bit = REPORT2_MIN_VALID_BIT;
    byte = REPORT2_MIN_VALID_BYTE;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
    TEST_ASSERT_EQUAL_UINT( CONSUMER_PAGE, page );
    TEST_ASSERT_EQUAL_UINT( 0x0A, header );
    TEST_ASSERT_EQUAL_UINT( 0x26, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0x02, data[ 1 ]);

    reportId = 3;
    bit = REPORT3_MIN_VALID_BIT;
    byte = REPORT3_MIN_VALID_BYTE;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
    TEST_ASSERT_EQUAL_UINT( TELEPHONY_DEVICE_PAGE, page );
    TEST_ASSERT_EQUAL_UINT( 0x09, header );
    TEST_ASSERT_EQUAL_UINT( 0x20, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0x00, data[ 1 ]);
}

void test_invalid_report_id( void )
{
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ]  = { 0xBA, 0xD2 };
    unsigned char header = 0x33;
    unsigned char page = 0x44;

    unsigned reportId = 0;
    unsigned bit = REPORT1_MIN_VALID_BIT;
    unsigned byte = REPORT1_MIN_VALID_BYTE;
    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_ID, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0x33, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD2, data[ 1 ]);
}

void test_unused_report_id( void )
{
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ]  = { 0xBA, 0xD2 };
    unsigned char header = 0x33;
    unsigned char page = 0x44;

    unsigned reportId = 8;
    unsigned bit = REPORT1_MIN_VALID_BIT;
    unsigned byte = REPORT1_MIN_VALID_BYTE;
    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_ID, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0x33, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD2, data[ 1 ]);
}

void test_overflow_bit_hidGetReportItem( void )
{
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xBA, 0xD1 };
    unsigned char header = 0xAA;
    unsigned char page = 0x44;

    unsigned reportId = 1;
    unsigned bit = REPORT1_MAX_VALID_BIT + 1;
    unsigned byte = REPORT1_MAX_VALID_BYTE;
    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);

    reportId = 2;
    bit = REPORT2_MAX_VALID_BIT + 1;
    byte = REPORT2_MAX_VALID_BYTE;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);

    reportId = 3;
    bit = REPORT3_MAX_VALID_BIT + 1;
    byte = REPORT3_MAX_VALID_BYTE;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);
}

void test_overflow_byte_hidGetReportItem( void )
{
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xBA, 0xD1 };
    unsigned char header = 0xAA;
    unsigned char page = 0x44;

    unsigned reportId = 1;
    unsigned bit = REPORT1_MAX_VALID_BIT;
    unsigned byte = REPORT1_MAX_VALID_BYTE + 1;
    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);

    reportId = 2;
    bit = REPORT2_MAX_VALID_BIT;
    byte = REPORT2_MAX_VALID_BYTE + 1;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);

    reportId = 3;
    bit = REPORT3_MAX_VALID_BIT;
    byte = REPORT3_MAX_VALID_BYTE + 1;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);
}

void test_underflow_bit_hidGetReportItem( void )
{
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xBA, 0xD1 };
    unsigned char header = 0xAA;
    unsigned char page = 0x44;

    unsigned reportId = 1;
    unsigned bit = REPORT1_MIN_VALID_BIT - 1;
    unsigned byte = REPORT1_MIN_VALID_BYTE;
    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);

    reportId = 2;
    bit = REPORT2_MIN_VALID_BIT - 1;
    byte = REPORT2_MIN_VALID_BYTE;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);

    reportId = 3;
    bit = REPORT3_MIN_VALID_BIT - 1;
    byte = REPORT3_MIN_VALID_BYTE;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);
}

void test_underflow_byte_hidGetReportItem( void )
{
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xBA, 0xD1 };
    unsigned char header = 0xAA;
    unsigned char page = 0x44;

    unsigned reportId = 1;
    unsigned bit = REPORT1_MIN_VALID_BIT;
    unsigned byte = REPORT1_MIN_VALID_BYTE - 1;
    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);

    reportId = 2;
    bit = REPORT2_MIN_VALID_BIT;
    byte = REPORT2_MIN_VALID_BYTE - 1;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);

    reportId = 3;
    bit = REPORT3_MIN_VALID_BIT;
    byte = REPORT3_MIN_VALID_BYTE - 1;
    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);
}

// Configurable and non-configurable item tests
void test_configurable_item_hidSetReportItem( void )
{
    const unsigned reportId = 1;
    const unsigned bit = REPORT1_MIN_VALID_BIT;
    const unsigned byte = REPORT1_MIN_VALID_BYTE;
    unsigned char data[ 1 ] = { KEYBOARD_X };
    unsigned char header = construct_usage_header( sizeof( data ) / sizeof( unsigned char ));
    unsigned char page = KEYBOARD_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
    TEST_ASSERT_EQUAL_UINT( KEYBOARD_PAGE, page );
    TEST_ASSERT_EQUAL_UINT( 0x09, header );
    TEST_ASSERT_EQUAL_UINT( KEYBOARD_X, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0x00, data[ 1 ]);
}

// Testing that the high byte of the report gets correctly cleared
void test_configurable_item_hidSetReportItem_multibyte_orig( void )
{
    const unsigned reportId = 2;
    const unsigned bit = 1;  // This byte&bit combo is originally set be 2 bytes long in the header
    const unsigned byte = 0;
    unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    unsigned char header = construct_usage_header( sizeof( data ) / sizeof( unsigned char ));
    unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
    TEST_ASSERT_EQUAL_UINT( CONSUMER_PAGE, page );
    TEST_ASSERT_EQUAL_UINT( 0x09, header );
    TEST_ASSERT_EQUAL_UINT( LOUDNESS_CONTROL, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0x00, data[ 1 ]);
}

void test_nonconfigurable_item_hidSetReportItem( void )
{
    const unsigned reportId = 1;
    const unsigned bit = 1;     // This bit and byte combination should not appear in the
    const unsigned byte = 0;    // hidConfigurableElements list in hid_report_descriptors.c.
    const unsigned char data[ 1 ] = { KEYBOARD_X };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = KEYBOARD_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

// Bit range tests
void test_max_bit_hidSetReportItem( void )
{
    const unsigned char header = construct_usage_header( 0 );

    unsigned reportId = 1;
    unsigned bit = REPORT1_MAX_VALID_BIT;  // See the hidConfigurableElements list in hid_report_descriptors.c.
    unsigned byte = REPORT1_MAX_VALID_BYTE;
    unsigned char page = KEYBOARD_PAGE;
    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    reportId = 2;
    bit = REPORT2_MAX_VALID_BIT;
    byte = REPORT2_MAX_VALID_BYTE;
    page = CONSUMER_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    reportId = 3;
    bit = REPORT3_MAX_VALID_BIT;
    byte = REPORT3_MAX_VALID_BYTE;
    page = TELEPHONY_DEVICE_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_min_bit_hidSetReportItem( void )
{
    const unsigned char header = construct_usage_header( 0 );

    unsigned reportId = 1;
    unsigned bit = REPORT1_MIN_VALID_BIT;  // See the hidConfigurableElements list in hid_report_descriptors.c.
    unsigned byte = REPORT1_MIN_VALID_BYTE;
    unsigned char page = KEYBOARD_PAGE;
    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    reportId = 2;
    bit = REPORT2_MIN_VALID_BIT;
    byte = REPORT2_MIN_VALID_BYTE;
    page = CONSUMER_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    reportId = 3;
    bit = REPORT3_MIN_VALID_BIT;
    byte = REPORT3_MIN_VALID_BYTE;
    page = TELEPHONY_DEVICE_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_overflow_bit_hidSetReportItem( void )
{
    const unsigned char header = construct_usage_header( 0 );

    unsigned reportId = 1;
    unsigned bit = REPORT1_MAX_VALID_BIT + 1;  // See the hidConfigurableElements list in hid_report_descriptors.c.
    unsigned byte = REPORT1_MAX_VALID_BYTE;
    unsigned char page = KEYBOARD_PAGE;
    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );

    reportId = 2;
    bit = REPORT2_MAX_VALID_BIT + 1;
    byte = REPORT2_MAX_VALID_BYTE;
    page = CONSUMER_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );

    reportId = 3;
    bit = REPORT3_MAX_VALID_BIT + 1;
    byte = REPORT3_MAX_VALID_BYTE;
    page = TELEPHONY_DEVICE_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

void test_underflow_bit_hidSetReportItem( void )
{
    const unsigned char header = construct_usage_header( 0 );

    unsigned reportId = 1;
    unsigned bit = REPORT1_MIN_VALID_BIT - 1;  // See the hidConfigurableElements list in hid_report_descriptors.c.
    unsigned byte = REPORT1_MIN_VALID_BYTE;
    unsigned char page = KEYBOARD_PAGE;
    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );

    reportId = 2;
    bit = REPORT2_MIN_VALID_BIT - 1;
    byte = REPORT2_MIN_VALID_BYTE;
    page = CONSUMER_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );

    reportId = 3;
    bit = REPORT3_MIN_VALID_BIT - 1;
    byte = REPORT3_MIN_VALID_BYTE;
    page = TELEPHONY_DEVICE_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

void test_overflow_byte_hidSetReportItem( void )
{
    const unsigned char header = construct_usage_header( 0 );

    unsigned reportId = 1;
    unsigned bit = REPORT1_MAX_VALID_BIT;  // See the hidConfigurableElements list in hid_report_descriptors.c.
    unsigned byte = REPORT1_MAX_VALID_BYTE + 1;
    unsigned char page = KEYBOARD_PAGE;
    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );

    reportId = 2;
    bit = REPORT2_MAX_VALID_BIT;
    byte = REPORT2_MAX_VALID_BYTE + 1;
    page = CONSUMER_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );

    reportId = 3;
    bit = REPORT3_MAX_VALID_BIT;
    byte = REPORT3_MAX_VALID_BYTE + 1;
    page = TELEPHONY_DEVICE_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

void test_underflow_byte_hidSetReportItem( void )
{
    const unsigned char header = construct_usage_header( 0 );

    unsigned reportId = 1;
    unsigned bit = REPORT1_MIN_VALID_BIT;  // See the hidConfigurableElements list in hid_report_descriptors.c.
    unsigned byte = REPORT1_MIN_VALID_BYTE - 1;
    unsigned char page = KEYBOARD_PAGE;
    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );

    reportId = 2;
    bit = REPORT2_MIN_VALID_BIT;
    byte = REPORT2_MIN_VALID_BYTE - 1;
    page = CONSUMER_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );

    reportId = 3;
    bit = REPORT3_MIN_VALID_BIT;
    byte = REPORT3_MIN_VALID_BYTE - 1;
    page = TELEPHONY_DEVICE_PAGE;
    retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

// Size range tests
void test_max_size_hidSetReportItem( void )
{
    const unsigned reportId = 1;
    const unsigned bit = REPORT1_MIN_VALID_BIT;
    const unsigned byte = REPORT1_MIN_VALID_BYTE;
    const unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0x00 };
    const unsigned char header = construct_usage_header( HID_REPORT_ITEM_MAX_SIZE );
    const unsigned char page = KEYBOARD_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_min_size_hidSetReportItem( void )
{
    const unsigned reportId = 1;
    const unsigned bit = REPORT1_MIN_VALID_BIT;
    const unsigned byte = REPORT1_MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0x00 );
    const unsigned char page = KEYBOARD_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_unsupported_size_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = REPORT1_MIN_VALID_BIT;
    const unsigned byte = REPORT1_MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0x03 );
    const unsigned char page = KEYBOARD_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

// Combined function tests
void test_initial_modification_without_subsequent_preparation( void )
{
    const unsigned reportId = 2;
    const unsigned bit = REPORT2_MIN_VALID_BIT;
    const unsigned byte = REPORT2_MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );
}

void test_initial_modification_with_subsequent_preparation( void )
{
    const unsigned reportId = 2;
    const unsigned bit = REPORT2_MIN_VALID_BIT;
    const unsigned byte = REPORT2_MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );
}

void test_initial_modification_with_subsequent_verification_1( void )
{
    const unsigned reportId = 2;
    const unsigned bit = REPORT2_MIN_VALID_BIT;
    const unsigned byte = REPORT2_MIN_VALID_BYTE;

    unsigned char get_data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xFF, 0xFF };
    unsigned char get_header = 0xFF;
    unsigned char get_page = 0xFF;

    const unsigned char set_data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char set_header = construct_usage_header( sizeof set_data / sizeof( unsigned char ));
    const unsigned char set_page = CONSUMER_PAGE;

    unsigned setRetVal = hidSetReportItem( reportId, byte, bit, set_page, set_header, set_data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, setRetVal );

    unsigned getRetVal = hidGetReportItem( reportId, byte, bit, &get_page, &get_header, get_data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, getRetVal );
    TEST_ASSERT_EQUAL_UINT( set_page, get_page );
    TEST_ASSERT_EQUAL_UINT( set_header, get_header );
    TEST_ASSERT_EQUAL_UINT( set_data[ 0 ], get_data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0, get_data[ 1 ]); // Should be MSB of data from hidUsageByte0Bit0 in hid_report_descriptor.h
}

void test_initial_modification_with_subsequent_verification_2( void )
{
    const unsigned reportId = 3;
    const unsigned bit = REPORT3_MIN_VALID_BIT;
    const unsigned byte = REPORT3_MIN_VALID_BYTE;

    {
        unsigned char get_data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xFF, 0xFF };
        unsigned char get_header = 0xFF;
        unsigned char get_page = 0xFF;

        const unsigned char set_data[ 2 ] = {( PHONE_HOST_HOLD & 0x00FF ), (( PHONE_HOST_HOLD & 0xFF00 ) >> 8 )};
        const unsigned char set_header = construct_usage_header( sizeof set_data / sizeof( unsigned char ));
        const unsigned char set_page = TELEPHONY_DEVICE_PAGE;

        unsigned setRetVal = hidSetReportItem( reportId, byte, bit, set_page, set_header, set_data );
        TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, setRetVal );

        unsigned getRetVal = hidGetReportItem( reportId, byte, bit, &get_page, &get_header, get_data );
        TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, getRetVal );
        TEST_ASSERT_EQUAL_UINT( set_page, get_page );
        TEST_ASSERT_EQUAL_UINT( set_header, get_header );
        TEST_ASSERT_EQUAL_UINT( set_data[ 0 ], get_data[ 0 ]);
        TEST_ASSERT_EQUAL_UINT( set_data[ 1 ], get_data[ 1 ]);
    }

    {
        unsigned char get_data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xFF, 0xFF };
        unsigned char get_header = 0xFF;
        unsigned char get_page = 0xFF;

        const unsigned char set_data[ 1 ] = { PHONE_KEY_9 };
        const unsigned char set_header = construct_usage_header( sizeof set_data / sizeof( unsigned char ));
        const unsigned char set_page = TELEPHONY_DEVICE_PAGE;

        unsigned setRetVal = hidSetReportItem( reportId, byte, bit, set_page, set_header, set_data );
        TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, setRetVal );

        unsigned getRetVal = hidGetReportItem( reportId, byte, bit, &get_page, &get_header, get_data );
        TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, getRetVal );
        TEST_ASSERT_EQUAL_UINT( set_page, get_page );
        TEST_ASSERT_EQUAL_UINT( set_header, get_header );
        TEST_ASSERT_EQUAL_UINT( set_data[ 0 ], get_data[ 0 ]);
        TEST_ASSERT_EQUAL_UINT( 0, get_data[ 1 ]); // The call to hidSetReportItem with size 1 in the header should return the MSB to zero
    }
}

//setIdle and associated timing functionality tests
void test_set_idle( void )
{
    unsigned reportId = 1;
    unsigned reportId2 = 2;

    unsigned setIdle = hidIsIdleActive( reportId );
    TEST_ASSERT_EQUAL_UINT( 0, setIdle );

    setIdle = hidIsIdleActive( reportId2 );
    TEST_ASSERT_EQUAL_UINT( 0, setIdle );

    hidSetIdle( reportId, 1 );
    setIdle = hidIsIdleActive( reportId );
    TEST_ASSERT_EQUAL_UINT( 1, setIdle );

    setIdle = hidIsIdleActive( reportId2 );
    TEST_ASSERT_EQUAL_UINT( 0, setIdle );
}

void test_set_all_idle( void )
{
    unsigned reportId = 1;
    unsigned reportId2 = 2;

    unsigned setIdle = hidIsIdleActive( reportId );
    TEST_ASSERT_EQUAL_UINT( 0, setIdle );

    setIdle = hidIsIdleActive( reportId2 );
    TEST_ASSERT_EQUAL_UINT( 0, setIdle );

    for ( reportId = 1; reportId <= HID_REPORT_COUNT; ++reportId ) {
        hidSetIdle( reportId, 1 );
        setIdle = hidIsIdleActive( reportId );
        TEST_ASSERT_EQUAL_UINT( 1, setIdle );
    }
}

void test_change_pending( void )
{
    unsigned reportId = 1;
    unsigned reportId2 = 2;

    unsigned changePending = hidIsChangePending( reportId );
    TEST_ASSERT_EQUAL_UINT( 0, changePending );

    changePending = hidIsChangePending( reportId2 );
    TEST_ASSERT_EQUAL_UINT( 0, changePending );

    hidSetChangePending( reportId );
    changePending = hidIsChangePending( reportId );
    TEST_ASSERT_EQUAL_UINT( 1, changePending );

    changePending = hidIsChangePending( reportId2 );
    TEST_ASSERT_EQUAL_UINT( 0, changePending );
}

void test_change_pending_all( void )
{
    unsigned reportId = 1;

    unsigned changePending = hidIsChangePending( reportId );
    TEST_ASSERT_EQUAL_UINT( 0, changePending );

    for ( reportId = 1; reportId <= HID_REPORT_COUNT; ++reportId ) {
        hidSetChangePending( reportId );
        changePending = hidIsChangePending( reportId );
        TEST_ASSERT_EQUAL_UINT( 1, changePending );
    }
}

void test_report_time( void )
{
    unsigned reportTime1 = 123;
    unsigned reportTime2 = 456;

    hidCaptureReportTime(1, reportTime1);
    hidCaptureReportTime(2, reportTime2);
    reportTime1 = hidGetReportTime(1);
    reportTime2 = hidGetReportTime(2);

    TEST_ASSERT_EQUAL_UINT(123, reportTime1);
    TEST_ASSERT_EQUAL_UINT(456, reportTime2);
}

void test_report_time_calc( void )
{
    unsigned reportTime1 = 123;
    unsigned reportTime2 = 456;
    unsigned reportPeriod1 = 10;
    unsigned reportPeriod2 = 5;

    hidCaptureReportTime(1, reportTime1);
    hidCaptureReportTime(2, reportTime2);
    hidSetReportPeriod(1, reportPeriod1);
    hidSetReportPeriod(2, reportPeriod2);
    reportTime1 = hidGetReportTime(1);
    reportTime2 = hidGetReportTime(2);
    reportPeriod1 = hidGetReportPeriod(1);
    reportPeriod2 = hidGetReportPeriod(2);

    TEST_ASSERT_EQUAL_UINT(123, reportTime1);
    TEST_ASSERT_EQUAL_UINT(456, reportTime2);
    TEST_ASSERT_EQUAL_UINT(10, reportPeriod1);
    TEST_ASSERT_EQUAL_UINT(5, reportPeriod2);

    hidCalcNextReportTime(1);
    hidCalcNextReportTime(2);
    unsigned nextReportTime1 = hidGetNextReportTime(1);
    unsigned nextReportTime2 = hidGetNextReportTime(2);
    TEST_ASSERT_EQUAL_UINT( reportTime1 + reportPeriod1, nextReportTime1 );
    TEST_ASSERT_EQUAL_UINT( reportTime2 + reportPeriod2, nextReportTime2 );
}

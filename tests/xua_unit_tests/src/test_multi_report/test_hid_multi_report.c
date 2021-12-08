// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>
#include <stdio.h>

#include "xua_unit_tests.h"
#include "xua_hid_report_descriptor.h"
#include "hid_report_descriptor.h"

#define HID_REPORT_ITEM_TYPE_GLOBAL     ( 0x01 )
#define HID_REPORT_ITEM_TYPE_LOCAL      ( 0x02 )
#define HID_REPORT_ITEM_TYPE_MAIN       ( 0x00 )
#define HID_REPORT_ITEM_TYPE_RESERVED   ( 0x03 )

#define KEYBOARD_PAGE           ( 0x07 )
#define CONSUMER_PAGE           ( 0x0C )
#define TELEPHONY_DEVICE_PAGE   ( 0x0B )
#define LOUDNESS_CONTROL        ( 0xE7 )
#define AL_CONTROL_PANEL        ( 0x019F )

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
    hidResetReportDescriptor();
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

#if 0
//here

// Configurable and non-configurable item tests
void xtest_configurable_item_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void xtest_nonconfigurable_item_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MAX_VALID_BIT;     // This bit and byte combination should not appear in the
    const unsigned byte = MIN_VALID_BYTE;    // hidConfigurableElements list in hid_report_descriptors.c.
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

// Bit range tests
void xtest_max_bit_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MAX_VALID_BIT;     // Only byte 1 has bit 7 not reserved,  See the
    const unsigned byte = MAX_VALID_BYTE;   // hidConfigurableElements list in hid_report_descriptors.c.
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void xtest_min_bit_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void xtest_overflow_bit_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MAX_VALID_BIT + 1;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

void xtest_underflow_bit_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const int bit = MIN_VALID_BIT - 1;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, ( unsigned ) bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

// Byte range tests
void xtest_max_byte_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MAX_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void xtest_min_byte_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void xtest_overflow_byte_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MAX_VALID_BYTE + 1;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

void xtest_underflow_byte_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const int byte = MIN_VALID_BYTE - 1;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId,  ( unsigned ) byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

// Size range tests
void xtest_max_size_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0x00 };
    const unsigned char header = construct_usage_header( HID_REPORT_ITEM_MAX_SIZE );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void xtest_min_size_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0x00 );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void xtest_unsupported_size_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0x03 );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

// Header tag and type tests
void xtest_bad_tag_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char good_header = construct_usage_header( 0x00 );
    const unsigned char page = CONSUMER_PAGE;

    for( unsigned tag = 0x01; tag <= 0x0F; ++tag ) {
        unsigned char bad_header = good_header | (( 0x0F << HID_REPORT_ITEM_HDR_TAG_SHIFT ) & HID_REPORT_ITEM_HDR_TAG_MASK );
        unsigned retVal = hidSetReportItem( reportId, byte, bit, page, bad_header, NULL );
        TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
    }
}

void xtest_global_type_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = ( construct_usage_header( 0x00 ) & ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
        (( HID_REPORT_ITEM_TYPE_GLOBAL << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

void xtest_local_type_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = ( construct_usage_header( 0x00 ) & ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
        (( HID_REPORT_ITEM_TYPE_LOCAL << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void xtest_main_type_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = ( construct_usage_header( 0x00 ) & ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
        (( HID_REPORT_ITEM_TYPE_MAIN << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

void xtest_reserved_type_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = ( construct_usage_header( 0x00 ) & ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
        (( HID_REPORT_ITEM_TYPE_RESERVED << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

// Combined function tests
void xtest_initial_modification_without_subsequent_preparation( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );
}

void xtest_initial_modification_with_subsequent_preparation( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );
}

void xtest_initial_modification_with_subsequent_verification_1( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;

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

void xtest_initial_modification_with_subsequent_verification_2( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;

    {
        unsigned char get_data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xFF, 0xFF };
        unsigned char get_header = 0xFF;
        unsigned char get_page = 0xFF;

        const unsigned char set_data[ 2 ] = {( AL_CONTROL_PANEL & 0x00FF ), (( AL_CONTROL_PANEL & 0xFF00 ) >> 8 )};
        const unsigned char set_header = construct_usage_header( sizeof set_data / sizeof( unsigned char ));
        const unsigned char set_page = CONSUMER_PAGE;

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
        TEST_ASSERT_EQUAL_UINT( 0, get_data[ 1 ]); // The call to hidSetReportItem with size 1 in the header should return the MSB to zero
    }
}

void xtest_modification_without_subsequent_preparation( void )
{
    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );

    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_PAGE;

    hidResetReportDescriptor();
    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );
}

void xtest_modification_with_subsequent_preparation( void )
{
    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );

    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_PAGE;

    hidResetReportDescriptor();
    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    hidPrepareReportDescriptor();
    reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );
}

#endif
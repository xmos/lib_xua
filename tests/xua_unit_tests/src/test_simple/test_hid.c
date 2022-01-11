// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>
#include <stdio.h>

#include "xua_unit_tests.h"
#include "xua_hid_report.h"

// Test constants related to the report descriptor defined in hid_report_descriptor.h
#define MAX_VALID_BIT   ( 7 )
#define MAX_VALID_BYTE  ( 1 )

#define MIN_VALID_BIT   ( 0 )
#define MIN_VALID_BYTE  ( 0 )

#define HID_REPORT_LENGTH  ( 2 )
#define HID_REPORT_COUNT   ( 1 )
#define HID_REPORTID_LIMIT ( 1 )

// Constants from the USB HID Usage Tables
#define CONSUMER_CONTROL_PAGE   ( 0x0C )
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
    hidReportInit();
    hidResetReportDescriptor();
}

void test_validate_report( void ) {
    unsigned retVal = hidReportValidate();
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_reportid_in_use( void ) {
    unsigned reportIdInUse = hidIsReportIdInUse();
    TEST_ASSERT_EQUAL_UINT( 0, reportIdInUse );
}

void test_get_next_valid_report_id( void ) {
    unsigned reportId = 0U;

    reportId = hidGetNextValidReportId(reportId);
    TEST_ASSERT_EQUAL_UINT( 0, reportId );

    reportId = hidGetNextValidReportId(reportId);
    TEST_ASSERT_EQUAL_UINT( 0, reportId );
}

void test_is_report_id_valid( void ) {
    unsigned isValid = 0;

    unsigned reportId = 0;
    isValid = hidIsReportIdValid( reportId );
    TEST_ASSERT_EQUAL_UINT( 1, isValid );

    reportId = 1;
    isValid = hidIsReportIdValid( reportId );
    TEST_ASSERT_EQUAL_UINT( 0, isValid );
}

// Basic report descriptor tests
void test_unprepared_hidGetReportDescriptor( void )
{
    const unsigned reportId = 0;
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );

    unsigned reportLength = hidGetReportLength( reportId );
    TEST_ASSERT_EQUAL_UINT( 0, reportLength );
}

void test_prepared_hidGetReportDescriptor( void )
{
    const unsigned reportId = 0;

    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );

    unsigned reportLength = hidGetReportLength( reportId );
    TEST_ASSERT_EQUAL_UINT( HID_REPORT_LENGTH, reportLength );
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
    const unsigned reportId = 0;
    const unsigned bit = MAX_VALID_BIT;
    const unsigned byte = MAX_VALID_BYTE;
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ];
    unsigned char header;
    unsigned char page;

    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
    TEST_ASSERT_EQUAL_UINT( CONSUMER_CONTROL_PAGE, page );
    TEST_ASSERT_EQUAL_UINT( 0x09, header );
    TEST_ASSERT_EQUAL_UINT( 0xEA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0x00, data[ 1 ]);
}

void test_min_loc_hidGetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ];
    unsigned char header;
    unsigned char page;

    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
    TEST_ASSERT_EQUAL_UINT( CONSUMER_CONTROL_PAGE, page );
    TEST_ASSERT_EQUAL_UINT( 0x09, header );
    TEST_ASSERT_EQUAL_UINT( 0xE2, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0x00, data[ 1 ]);
}

void test_overflow_bit_hidGetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MAX_VALID_BIT + 1;
    const unsigned byte = MAX_VALID_BYTE;
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xBA, 0xD1 };
    unsigned char header = 0xAA;
    unsigned char page = 0x44;

    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);
}

void test_overflow_byte_hidGetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MAX_VALID_BIT;
    const unsigned byte = MAX_VALID_BYTE + 1;
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xBA, 0xD1 };
    unsigned char header = 0xAA;
    unsigned char page = 0x44;

    unsigned retVal = hidGetReportItem( reportId, byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);
}

void test_underflow_bit_hidGetReportItem( void )
{
    const unsigned reportId = 0;
    const      int bit = MIN_VALID_BIT - 1;
    const unsigned byte = MIN_VALID_BYTE;
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xBA, 0xD1 };
    unsigned char header = 0xAA;
    unsigned char page = 0x44;

    unsigned retVal = hidGetReportItem( reportId, byte, ( unsigned ) bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);
}

void test_underflow_byte_hidGetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const      int byte = MIN_VALID_BYTE - 1;
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xBA, 0xD1 };
    unsigned char header = 0xAA;
    unsigned char page = 0x44;

    unsigned retVal = hidGetReportItem( reportId, ( unsigned ) byte, bit, &page, &header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
    TEST_ASSERT_EQUAL_UINT( 0x44, page );
    TEST_ASSERT_EQUAL_UINT( 0xAA, header );
    TEST_ASSERT_EQUAL_UINT( 0xBA, data[ 0 ]);
    TEST_ASSERT_EQUAL_UINT( 0xD1, data[ 1 ]);
}

// Configurable and non-configurable item tests
void test_configurable_item_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_nonconfigurable_item_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MAX_VALID_BIT;     // This bit and byte combination should not appear in the
    const unsigned byte = MIN_VALID_BYTE;    // hidConfigurableElements list in hid_report_descriptors.c.
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

// Bit range tests
void test_max_bit_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MAX_VALID_BIT;     // Only byte 1 has bit 7 not reserved,  See the
    const unsigned byte = MAX_VALID_BYTE;   // hidConfigurableElements list in hid_report_descriptors.c.
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_min_bit_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_overflow_bit_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MAX_VALID_BIT + 1;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

void test_underflow_bit_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const int bit = MIN_VALID_BIT - 1;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, ( unsigned ) bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

// Byte range tests
void test_max_byte_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MAX_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_min_byte_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_overflow_byte_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MAX_VALID_BYTE + 1;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

void test_underflow_byte_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const int byte = MIN_VALID_BYTE - 1;
    const unsigned char header = construct_usage_header( 0 );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId,  ( unsigned ) byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_LOCATION, retVal );
}

// Size range tests
void test_max_size_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0x00 };
    const unsigned char header = construct_usage_header( HID_REPORT_ITEM_MAX_SIZE );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_min_size_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0x00 );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_unsupported_size_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = construct_usage_header( 0x03 );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

// Header tag and type tests
void test_bad_tag_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char good_header = construct_usage_header( 0x00 );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    for( unsigned tag = 0x01; tag <= 0x0F; ++tag ) {
        unsigned char bad_header = good_header | (( tag << HID_REPORT_ITEM_HDR_TAG_SHIFT ) & HID_REPORT_ITEM_HDR_TAG_MASK );
        unsigned retVal = hidSetReportItem( reportId, byte, bit, page, bad_header, NULL );
        TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
    }
}

void test_global_type_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = ( construct_usage_header( 0x00 ) & ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
        (( HID_REPORT_ITEM_TYPE_GLOBAL << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

void test_local_type_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = ( construct_usage_header( 0x00 ) & ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
        (( HID_REPORT_ITEM_TYPE_LOCAL << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );
}

void test_main_type_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = ( construct_usage_header( 0x00 ) & ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
        (( HID_REPORT_ITEM_TYPE_MAIN << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

void test_reserved_type_hidSetReportItem( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char header = ( construct_usage_header( 0x00 ) & ~HID_REPORT_ITEM_HDR_TYPE_MASK ) |
        (( HID_REPORT_ITEM_TYPE_RESERVED << HID_REPORT_ITEM_HDR_TYPE_SHIFT ) & HID_REPORT_ITEM_HDR_TYPE_MASK );
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, NULL );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_BAD_HEADER, retVal );
}

// Combined function tests
void test_initial_modification_without_subsequent_preparation( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );
}

void test_initial_modification_with_subsequent_preparation( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );
}

void test_initial_modification_with_subsequent_verification_1( void )
{
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;

    unsigned char get_data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xFF, 0xFF };
    unsigned char get_header = 0xFF;
    unsigned char get_page = 0xFF;

    const unsigned char set_data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char set_header = construct_usage_header( sizeof set_data / sizeof( unsigned char ));
    const unsigned char set_page = CONSUMER_CONTROL_PAGE;

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
    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;

    {
        unsigned char get_data[ HID_REPORT_ITEM_MAX_SIZE ] = { 0xFF, 0xFF };
        unsigned char get_header = 0xFF;
        unsigned char get_page = 0xFF;

        const unsigned char set_data[ 2 ] = {( AL_CONTROL_PANEL & 0x00FF ), (( AL_CONTROL_PANEL & 0xFF00 ) >> 8 )};
        const unsigned char set_header = construct_usage_header( sizeof set_data / sizeof( unsigned char ));
        const unsigned char set_page = CONSUMER_CONTROL_PAGE;

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
        const unsigned char set_page = CONSUMER_CONTROL_PAGE;

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

void test_modification_without_subsequent_preparation( void )
{
    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );

    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    hidResetReportDescriptor();
    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );
}

void test_modification_with_subsequent_preparation( void )
{
    hidPrepareReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );

    const unsigned reportId = 0;
    const unsigned bit = MIN_VALID_BIT;
    const unsigned byte = MIN_VALID_BYTE;
    const unsigned char data[ 1 ] = { LOUDNESS_CONTROL };
    const unsigned char header = construct_usage_header( sizeof data / sizeof( unsigned char ));
    const unsigned char page = CONSUMER_CONTROL_PAGE;

    hidResetReportDescriptor();
    unsigned retVal = hidSetReportItem( reportId, byte, bit, page, header, data );
    TEST_ASSERT_EQUAL_UINT( HID_STATUS_GOOD, retVal );

    hidPrepareReportDescriptor();
    reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );
}

//setIdle functionality tests
void test_set_idle( void )
{
    unsigned reportId = 0;

    unsigned setIdle = hidIsIdleActive( reportId );
    TEST_ASSERT_EQUAL_UINT( 0, setIdle );

    hidSetIdle( reportId, 1 );
    setIdle = hidIsIdleActive( reportId );
    TEST_ASSERT_EQUAL_UINT( 1, setIdle );
}

void test_change_pending( void )
{
    unsigned reportId = 0;

    unsigned changePending = hidIsChangePending( reportId );
    TEST_ASSERT_EQUAL_UINT( 0, changePending );

    hidSetChangePending( reportId );
    changePending = hidIsChangePending( reportId );
    TEST_ASSERT_EQUAL_UINT( 1, changePending );

    hidClearChangePending( reportId );
    changePending = hidIsChangePending( reportId );
    TEST_ASSERT_EQUAL_UINT( 0, changePending );
}

void test_report_time( void )
{
    unsigned reportTime = 123;
    unsigned reportPeriod = 10;

    hidSetReportPeriod(0, reportPeriod);
    hidCaptureReportTime(0, reportTime);
    reportTime = hidGetReportTime(0);
    reportPeriod = hidGetReportPeriod(0);

    TEST_ASSERT_EQUAL_UINT(123, reportTime);
    TEST_ASSERT_EQUAL_UINT(10, reportPeriod);

    hidCalcNextReportTime(0);

    unsigned nextReportTime = hidGetNextReportTime(0);
    TEST_ASSERT_EQUAL_UINT(133, nextReportTime);
}

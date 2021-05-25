// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <assert.h>
#include <stddef.h>
#include <xs1.h>
#include "xua_hid_report_descriptor.h"

#define HID_REPORT_ITEM_LOC_BIT_MASK   ( 0x70 )
#define HID_REPORT_ITEM_LOC_BIT_SHIFT  ( 4 )

#define HID_REPORT_ITEM_LOC_BYTE_MASK  ( 0x0F )
#define HID_REPORT_ITEM_LOC_BYTE_SHIFT ( 0 )

#if 0
/* Existing static report descriptor kept for reference */
unsigned char hidReportDescriptor[] =
{
    0x05, 0x01,         /* Usage Page (Generic Desktop) */
    0x09, 0x06,         /* Usage (Keyboard) */
    0xa1, 0x01,         /* Collection (Application) */
    0x75, 0x01,         /* Report Size (1) */
    0x95, 0x04,         /* Report Count (4) */
    0x15, 0x00,         /* Logical Minimum (0) */
    0x25, 0x00,         /* Logical Maximum (0) */
    0x81, 0x01,         /* Input (Cnst, Ary, Abs, No Wrap, Lin, Pref, No Nul) */
    0x95, 0x01,         /* Report Count (1) */
    0x25, 0x01,         /* Logical Maximum (1) */
    0x05, 0x07,         /* Usage Page (Key Codes) */
    0x19, 0x17,         /* Usage Minimum (Keyboard t or T) */
    0x29, 0x17,         /* Usage Maximum (Keyboard t or T) */
    0x81, 0x02,         /* Input (Data, Var, Abs, No Wrap, Lin, Pref, No Nul) */
    0x05, 0x0C,         /* Usage Page (Consumer) */
    0x0a, 0x26, 0x02,   /* Usage (AC Stop) */
    0x81, 0x02,         /* Input (Data, Var, Abs, No Wrap, Lin, Pref, No Nul) */
    0x95, 0x02,         /* Report Count (2) */
    0x05, 0x07,         /* Usage Page (Key Codes) */
    0x19, 0x72,         /* Usage Minimum (Keyboard F23) */
    0x29, 0x73,         /* Usage Maximum (Keyboard F24) */
    0x81, 0x02,         /* Input (Data, Var, Abs, No Wrap, Lin, Pref, No Nul) */
    0xc0                /* End collection (Application) */
};
#endif

static const USB_HID_Short_Item_t hidCollectionApplication  = { .header = 0xA1, .data = { 0x01, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidCollectionEnd          = { .header = 0xC0, .data = { 0x00, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidCollectionLogical      = { .header = 0xA1, .data = { 0x02, 0x00 }, .location = 0x00 };

static const USB_HID_Short_Item_t hidInputConstArray        = { .header = 0x81, .data = { 0x01, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidInputDataVar           = { .header = 0x81, .data = { 0x02, 0x00 }, .location = 0x00 };

static const USB_HID_Short_Item_t hidLogicalMaximum0        = { .header = 0x25, .data = { 0x00, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidLogicalMaximum1        = { .header = 0x25, .data = { 0x01, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidLogicalMinimum0        = { .header = 0x15, .data = { 0x00, 0x00 }, .location = 0x00 };

static const USB_HID_Short_Item_t hidReportCount1           = { .header = 0x95, .data = { 0x01, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidReportCount4           = { .header = 0x95, .data = { 0x04, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidReportCount6           = { .header = 0x95, .data = { 0x06, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidReportSize1            = { .header = 0x75, .data = { 0x01, 0x00 }, .location = 0x00 };

static const USB_HID_Short_Item_t hidUsageConsumerControl   = { .header = 0x09, .data = { 0x01, 0x00 }, .location = 0x00 };

static const USB_HID_Short_Item_t hidUsagePageConsumer      = { .header = 0x05, .data = { 0x0C, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidUsagePageKeyboard      = { .header = 0x05, .data = { 0x07, 0x00 }, .location = 0x00 };
static const USB_HID_Short_Item_t hidUsagePageTelephony     = { .header = 0x05, .data = { 0x0B, 0x00 }, .location = 0x00 };

static USB_HID_Short_Item_t hidUsageByte0Bit3   = { .header = 0x09, .data = { 0x73, 0x00 }, .location = 0x30 }; // F24
static USB_HID_Short_Item_t hidUsageByte0Bit2   = { .header = 0x09, .data = { 0x72, 0x00 }, .location = 0x20 }; // F23
static USB_HID_Short_Item_t hidUsageByte0Bit0   = { .header = 0x09, .data = { 0x17, 0x00 }, .location = 0x00 }; // 't'

static USB_HID_Short_Item_t hidUsageByte1Bit7   = { .header = 0x09, .data = { 0xEA, 0x00 }, .location = 0x71 }; // Vol-
static USB_HID_Short_Item_t hidUsageByte1Bit6   = { .header = 0x09, .data = { 0xE9, 0x00 }, .location = 0x61 }; // Vol+
static USB_HID_Short_Item_t hidUsageByte1Bit4   = { .header = 0x09, .data = { 0x00, 0x00 }, .location = 0x41 }; // Voice Command
static USB_HID_Short_Item_t hidUsageByte1Bit2   = { .header = 0x09, .data = { 0xE2, 0x00 }, .location = 0x21 }; // Mute
static USB_HID_Short_Item_t hidUsageByte1Bit1   = { .header = 0x09, .data = { 0x00, 0x00 }, .location = 0x11 }; // AC Search
static USB_HID_Short_Item_t hidUsageByte1Bit0   = { .header = 0x09, .data = { 0x00, 0x00 }, .location = 0x01 }; // AC Stop

static USB_HID_Short_Item_t hidUsageByte2Bit1   = { .header = 0x09, .data = { 0x2F, 0x00 }, .location = 0x12 }; // Phone Mute
static USB_HID_Short_Item_t hidUsageByte2Bit0   = { .header = 0x09, .data = { 0x20, 0x00 }, .location = 0x02 }; // Hook Switch

static USB_HID_Short_Item_t* const hidConfigurableItems[] = {
    &hidUsageByte0Bit0,
    &hidUsageByte0Bit2,
    &hidUsageByte0Bit3,
    &hidUsageByte1Bit0,
    &hidUsageByte1Bit1,
    &hidUsageByte1Bit2,
    &hidUsageByte1Bit4,
    &hidUsageByte1Bit6,
    &hidUsageByte1Bit7,
    &hidUsageByte2Bit0,
    &hidUsageByte2Bit1
};

static const USB_HID_Short_Item_t* const hidReportDescriptorItems[] = {
    &hidUsagePageConsumer,
    &hidUsageConsumerControl,
    &hidCollectionApplication,
        &hidReportSize1,
        &hidLogicalMinimum0,
        &hidCollectionLogical,      // Byte 0
            &hidUsagePageKeyboard,
            &hidLogicalMaximum1,
            &hidReportCount1,
            &hidUsageByte0Bit0,
            &hidInputDataVar,
            &hidLogicalMaximum0,
            &hidInputConstArray,
            &hidLogicalMaximum1,
            &hidUsageByte0Bit2,
            &hidInputDataVar,
            &hidUsageByte0Bit3,
            &hidInputDataVar,
            &hidLogicalMaximum0,
            &hidReportCount4,
            &hidInputConstArray,
        &hidCollectionEnd,
        &hidCollectionLogical,      // Byte 1
            &hidUsagePageConsumer,
            &hidLogicalMaximum1,
            &hidReportCount1,
            &hidUsageByte1Bit0,
            &hidInputDataVar,
            &hidUsageByte1Bit1,
            &hidInputDataVar,
            &hidUsageByte1Bit2,
            &hidInputDataVar,
            &hidLogicalMaximum0,
            &hidInputConstArray,
            &hidLogicalMaximum1,
            &hidUsageByte1Bit4,
            &hidInputDataVar,
            &hidLogicalMaximum0,
            &hidInputConstArray,
            &hidLogicalMaximum1,
            &hidUsageByte1Bit6,
            &hidInputDataVar,
            &hidUsageByte1Bit7,
            &hidInputDataVar,
        &hidCollectionEnd,
        &hidCollectionLogical,      // Byte 2
            &hidUsagePageTelephony,
            &hidUsageByte2Bit0,
            &hidInputDataVar,
            &hidUsageByte2Bit1,
            &hidLogicalMaximum0,
            &hidReportCount6,
            &hidInputConstArray,
        &hidCollectionEnd,
    &hidCollectionEnd
};

#define HID_REPORT_ITEM_LOCATION_SIZE ( 1 )
#define HID_REPORT_DESCRIPTOR_MAX_LENGTH ( sizeof hidReportDescriptorItems / sizeof ( USB_HID_Short_Item_t ) * \
                                           ( sizeof ( USB_HID_Short_Item_t ) - HID_REPORT_ITEM_LOCATION_SIZE ))

static unsigned char hidReportDescriptor[ HID_REPORT_DESCRIPTOR_MAX_LENGTH ];
static size_t hidReportDescriptorLength = 0;
static unsigned hidReportDescriptorPrepared = 0;

/**
 * @brief Get the bit position from the location of an Item
 *
 * Parameters:
 *
 *  @param[in] location     The \c location field from a \c USB_HID_Short_Item
 *
 * @return The bit position of the Item
 */
static unsigned hidGetItemBitLocation( const unsigned char header );

/**
 * @brief Get the byte position from the location of an Item
 *
 * Parameters:
 *
 *  @param[in] location     The \c location field from a \c USB_HID_Short_Item
 *
 * @return The byte position of the Item within the HID Report
 */
static unsigned hidGetItemByteLocation( const unsigned char header );

/**
 * @brief Get the number of data bytes from the header of an Item
 *
 * Parameters:
 *
 *  @param[in] header  The \c header field from a \c USB_HID_Short_Item
 *
 * @return The amount of data for the Item
 */
static unsigned hidGetItemSize( const unsigned char header );

/**
 * @brief Get the Tag from the header of an Item
 *
 * Parameters:
 *
 *  @param[in] header  The \c header field from a \c USB_HID_Short_Item
 *
 * @return The Tag of the Item
 */
static unsigned hidGetItemTag( const unsigned char header );

/**
 * @brief Get the Type from the header of an Item
 *
 * Parameters:
 *
 *  @param[in] header  The \c header field from a \c USB_HID_Short_Item
 *
 * @return The Type of the Item
 */
static unsigned hidGetItemType( const unsigned char header );

/**
 * @brief Translate an Item from the \c USB_HID_Short_Item format to raw bytes
 *
 * Parameters:
 *
 *  @param[in]     inPtr        A pointer to a \c USB_HID_Short_Item
 *  @param[in,out] outPtrPtr    A pointer to a pointer to the next available space in the raw
 *                              byte buffer.  Passed as a pointer to a pointer to allow this
 *                              function to return the updated pointer to the raw byte buffer.
 *
 * @return The number of bytes placed in the raw byte buffer
 */
static size_t hidTranslateItem( const USB_HID_Short_Item_t* inPtr, unsigned char** outPtrPtr );


static unsigned hidGetItemBitLocation( const unsigned char location )
{
    unsigned bBit = ( location & HID_REPORT_ITEM_LOC_BIT_MASK ) >> HID_REPORT_ITEM_LOC_BIT_SHIFT;
    return bBit;
}

static unsigned hidGetItemByteLocation( const unsigned char location )
{
    unsigned bByte = ( location & HID_REPORT_ITEM_LOC_BYTE_MASK ) >> HID_REPORT_ITEM_LOC_BYTE_SHIFT;
    return bByte;
}

static unsigned hidGetItemSize( const unsigned char header )
{
    unsigned bSize = ( header & HID_REPORT_ITEM_HDR_SIZE_MASK ) >> HID_REPORT_ITEM_HDR_SIZE_SHIFT;
    return bSize;
}

static unsigned hidGetItemTag( const unsigned char header )
{
    unsigned bTag = ( header & HID_REPORT_ITEM_HDR_TAG_MASK ) >> HID_REPORT_ITEM_HDR_TAG_SHIFT;
    return bTag;
}

static unsigned hidGetItemType( const unsigned char header )
{
    unsigned bType = ( header & HID_REPORT_ITEM_HDR_TYPE_MASK ) >> HID_REPORT_ITEM_HDR_TYPE_SHIFT;
    return bType;
}

unsigned char* hidGetReportDescriptor( void )
{
    unsigned char* retVal = NULL;

    if( hidReportDescriptorPrepared ) {
        retVal = hidReportDescriptor;
    }

    return retVal;
}

size_t hidGetReportDescriptorLength( void )
{
    return ( hidReportDescriptorPrepared ) ? hidReportDescriptorLength : 0;
}

void hidPrepareReportDescriptor( void )
{
    if( !hidReportDescriptorPrepared ) {
        hidReportDescriptorLength = 0;
        unsigned char* ptr = hidReportDescriptor;
        for( unsigned idx = 0; idx < sizeof hidReportDescriptorItems / sizeof( USB_HID_Short_Item_t ); ++idx ) {
            hidReportDescriptorLength += hidTranslateItem( hidReportDescriptorItems[ idx ], &ptr );
        }

        hidReportDescriptorPrepared = 1;
    }
}

unsigned hidSetReportItem( const unsigned byte, const unsigned bit, const unsigned char header, const unsigned char data[] )
{
    unsigned retVal = HID_STATUS_BAD_LOCATION;
    unsigned bSize = hidGetItemSize( header );
    unsigned bTag  = hidGetItemTag ( header );
    unsigned bType = hidGetItemType( header );

    if(( HID_REPORT_ITEM_MAX_SIZE   <  bSize ) ||
       ( HID_REPORT_ITEM_USAGE_TAG  != bTag  ) ||
       ( HID_REPORT_ITEM_USAGE_TYPE != bType )) {
        retVal = HID_STATUS_BAD_HEADER;
    } else {
        for( unsigned itemIdx = 0; itemIdx < sizeof hidConfigurableItems / sizeof( USB_HID_Short_Item_t ); ++itemIdx ) {
            USB_HID_Short_Item_t item = *hidConfigurableItems[ itemIdx ];
            unsigned bBit  = hidGetItemBitLocation(  item.location );
            unsigned bByte = hidGetItemByteLocation( item.location );

            if(( bit == bBit ) && ( byte == bByte )) {
                item.header = header;

                for( unsigned dataIdx = 0; dataIdx < bSize; ++dataIdx ) {
                    item.data[ dataIdx ] = data[ dataIdx ];
                }

                *hidConfigurableItems[ itemIdx ] = item;
                hidReportDescriptorPrepared = 0;
                retVal = HID_STATUS_GOOD;
            }
        }
    }

    return retVal;
}

static size_t hidTranslateItem( const USB_HID_Short_Item_t* inPtr, unsigned char** outPtrPtr )
{
    size_t count = 0;
    *(*outPtrPtr)++ = inPtr->header;

    unsigned dataLength = hidGetItemSize( inPtr->header );
    for( unsigned idx = 0; idx < dataLength; ++idx ) {
        *(*outPtrPtr)++ = inPtr->data[ idx ];
        ++count;
    }

    return count;
}

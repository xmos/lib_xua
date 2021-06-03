// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <assert.h>
#include <stddef.h>
#include <xs1.h>
#include "xua_hid_report_descriptor.h"
#include "hid_report_descriptor.h"

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

size_t hidGetReportLength( void )
{
    return ( hidReportDescriptorPrepared ) ? HID_REPORT_LENGTH : 0;
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

void hidResetReportDescriptor( void )
{
    hidReportDescriptorPrepared = 0;
}

unsigned hidSetReportItem( const unsigned byte, const unsigned bit, const unsigned char header, const unsigned char data[] )
{
    unsigned retVal = HID_STATUS_IN_USE;

    if( !hidReportDescriptorPrepared ) {
        retVal = HID_STATUS_BAD_LOCATION;
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
                    retVal = HID_STATUS_GOOD;
                }
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

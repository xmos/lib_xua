// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <assert.h>
#include <stddef.h>
#include <xs1.h>
#include "xua_hid_report_descriptor.h"
#include "hid_report_descriptor.h"

#include <stdio.h>

#define HID_REPORT_ITEM_LOCATION_SIZE ( 1 )
#define HID_REPORT_DESCRIPTOR_ITEM_COUNT ( sizeof hidReportDescriptorItems / sizeof ( USB_HID_Short_Item_t* ))
#define HID_REPORT_DESCRIPTOR_MAX_LENGTH ( HID_REPORT_DESCRIPTOR_ITEM_COUNT * \
                                           ( sizeof ( USB_HID_Short_Item_t ) - HID_REPORT_ITEM_LOCATION_SIZE ))

static unsigned char hidReportDescriptor[ HID_REPORT_DESCRIPTOR_MAX_LENGTH ];
static size_t hidReportDescriptorLength = 0U;
static unsigned hidReportDescriptorPrepared = 0U;

/**
 * @brief Get the bit position from the location of a report element
 *
 * Parameters:
 *
 *  @param[in] location     The \c location field from a \c USB_HID_Report_Element_t
 *
 * @return The bit position of the report element
 */
static unsigned hidGetElementBitLocation( const unsigned short location );

/**
 * @brief Get the byte position from the location of a report element
 *
 * Parameters:
 *
 *  @param[in] location     The \c location field from a \c USB_HID_Report_Element_t
 *
 * @return The byte position of the report element within the HID report
 */
static unsigned hidGetElementByteLocation( const unsigned short location );

/**
 * @brief Get the report identifier from the location of a report element
 *
 * Parameters:
 *
 *  @param[in] location     The \c location field from a \c USB_HID_Report_Element_t
 *
 * @return The report id of the report element within the HID report
 */
static unsigned hidGetElementReportId( const unsigned short location );

/**
 * @brief Get the report length from the location of a report element
 *
 * Parameters:
 *
 *  @param[in] location     The \c location field from a \c USB_HID_Report_Element_t
 *
 * @return The length of the HID report
 */
static size_t hidGetElementReportLength( const unsigned short location );

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
 * @brief Get the Usage Page number for a given byte in the HID Report
 *
 * Parameters:
 *
 *  @param[in] id   The HID Report ID for the Usage Page.
 *                  A value of zero means the application does not use Report IDs.
 *
 * @return The USB HID Usage Page code or zero if the \a id parameter is out-of-range
 */
static unsigned hidGetUsagePage( const unsigned id );

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


static unsigned hidGetElementBitLocation( const unsigned short location )
{
    unsigned bBit = ( location & HID_REPORT_ELEMENT_LOC_BIT_MASK ) >> HID_REPORT_ELEMENT_LOC_BIT_SHIFT;
    return bBit;
}

static unsigned hidGetElementByteLocation( const unsigned short location )
{
    unsigned bByte = ( location & HID_REPORT_ELEMENT_LOC_BYTE_MASK ) >> HID_REPORT_ELEMENT_LOC_BYTE_SHIFT;
    return bByte;
}

static unsigned hidGetElementReportId( const unsigned short location )
{
    unsigned bReportId = ( location & HID_REPORT_ELEMENT_LOC_ID_MASK ) >> HID_REPORT_ELEMENT_LOC_ID_SHIFT;
    return bReportId;
}

static size_t hidGetElementReportLength( const unsigned short location )
{
    size_t bReportLen = (size_t)( location & HID_REPORT_ELEMENT_LOC_LEN_MASK ) >> HID_REPORT_ELEMENT_LOC_LEN_SHIFT;
    return bReportLen;
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
    size_t retVal = ( hidReportDescriptorPrepared ) ? hidReportDescriptorLength : 0U;
    return retVal;
}

unsigned hidGetReportIdLimit ( void ) {
  return HID_REPORT_COUNT;
}

#define HID_CONFIGURABLE_ELEMENT_COUNT ( sizeof hidConfigurableElements / sizeof ( USB_HID_Report_Element_t* ))
unsigned hidGetReportItem(
    const unsigned id,
    const unsigned byte,
    const unsigned bit,
    unsigned char* const page,
    unsigned char* const header,
    unsigned char data[]
)
{
    unsigned retVal = HID_STATUS_BAD_ID;
    for( size_t elementIdx = 0U; elementIdx < HID_CONFIGURABLE_ELEMENT_COUNT; ++elementIdx ) {
        USB_HID_Report_Element_t element = *hidConfigurableElements[ elementIdx ];
        unsigned bBit  = hidGetElementBitLocation( element.location );
        unsigned bByte = hidGetElementByteLocation( element.location );
        unsigned bId   = hidGetElementReportId( element.location );

        if( id == bId ) {
            retVal = HID_STATUS_BAD_LOCATION;

            if(( bit == bBit ) && ( byte == bByte )) {
                *page = hidGetUsagePage( id );
                *header = element.item.header;

                for( size_t dataIdx = 0U; dataIdx < HID_REPORT_ITEM_MAX_SIZE; ++data, ++dataIdx ) {
                    *data = element.item.data[ dataIdx ];
                }

                retVal = HID_STATUS_GOOD;
                break;
            }
        }
    }
    return retVal;
}

size_t hidGetReportLength( const unsigned id )
{
    size_t retVal = 0U;
    if( hidReportDescriptorPrepared ) {
        for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx ) {
            if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
                retVal = hidGetElementReportLength( hidReports[ idx ]->location );
            }
        }
    }
    return retVal;
}

static unsigned hidGetUsagePage( const unsigned id )
{
    unsigned retVal = 0U;
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            retVal = hidReports[ idx ]->item.data[ 0 ];
            break;
        }
    }
    return retVal;
}

void hidPrepareReportDescriptor( void )
{
    if( !hidReportDescriptorPrepared ) {
        hidReportDescriptorLength = 0U;
        unsigned char* ptr = hidReportDescriptor;

        for( size_t idx = 0U; idx < HID_REPORT_DESCRIPTOR_ITEM_COUNT; ++idx ) {
            hidReportDescriptorLength += hidTranslateItem( hidReportDescriptorItems[ idx ], &ptr );
        }

        hidReportDescriptorPrepared = 1U;
    }
}

void hidResetReportDescriptor( void )
{
    hidReportDescriptorPrepared = 0U;
}

unsigned hidSetReportItem(
    const unsigned id,
    const unsigned byte,
    const unsigned bit,
    const unsigned char page,
    const unsigned char header,
    const unsigned char data[]
)
{
    unsigned retVal = HID_STATUS_IN_USE;

    if( !hidReportDescriptorPrepared ) {
        retVal = HID_STATUS_BAD_ID;
        unsigned bSize = hidGetItemSize( header );
        unsigned bTag  = hidGetItemTag ( header );
        unsigned bType = hidGetItemType( header );

        if(( HID_REPORT_ITEM_MAX_SIZE   <  bSize ) ||
           ( HID_REPORT_ITEM_USAGE_TAG  != bTag  ) ||
           ( HID_REPORT_ITEM_USAGE_TYPE != bType )) {
            retVal = HID_STATUS_BAD_HEADER;
        } else {
            for( size_t elementIdx = 0U; elementIdx < HID_CONFIGURABLE_ELEMENT_COUNT; ++elementIdx ) {
                USB_HID_Report_Element_t element = *hidConfigurableElements[ elementIdx ];
                unsigned bBit  = hidGetElementBitLocation( element.location );
                unsigned bByte = hidGetElementByteLocation( element.location );
                unsigned bId   = hidGetElementReportId( element.location );

                if( id == bId ) {
                    retVal = HID_STATUS_BAD_PAGE;
                    unsigned pg = hidGetUsagePage( id );

                    if( page == pg ) {
                        retVal = HID_STATUS_BAD_LOCATION;

                        if(( bit == bBit ) && ( byte == bByte )) {
                            element.item.header = header;

                            for( size_t dataIdx = 0U; dataIdx < bSize; ++dataIdx ) {
                                element.item.data[ dataIdx ] = data[ dataIdx ];
                            }

                            for( size_t dataIdx = bSize; dataIdx < HID_REPORT_ITEM_MAX_SIZE; ++dataIdx ) {
                                element.item.data[ dataIdx ] = 0U;
                            }

                            *hidConfigurableElements[ elementIdx ] = element;
                            retVal = HID_STATUS_GOOD;
                            break;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

static size_t hidTranslateItem( const USB_HID_Short_Item_t* inPtr, unsigned char** outPtrPtr )
{
    size_t count = 0U;
    *(*outPtrPtr)++ = inPtr->header;
    ++count;

    unsigned dataLength = hidGetItemSize( inPtr->header );
    for( size_t idx = 0U; idx < dataLength; ++idx ) {
        *(*outPtrPtr)++ = inPtr->data[ idx ];
        ++count;
    }

    return count;
}

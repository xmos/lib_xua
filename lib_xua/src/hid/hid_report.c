// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xua_conf_full.h"
#if XUA_HID_ENABLED

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <xs1.h>

#include "descriptor_defs.h"
#include "xua_hid_report.h"
#include "hid_report_descriptor.h"
#include "swlock.h"


#define HID_REPORT_ITEM_LOCATION_SIZE ( 1 )
#define HID_REPORT_DESCRIPTOR_ITEM_COUNT ( sizeof hidReportDescriptorItems / sizeof ( USB_HID_Short_Item_t* ))
#define HID_REPORT_DESCRIPTOR_MAX_LENGTH ( HID_REPORT_DESCRIPTOR_ITEM_COUNT * \
                                           ( sizeof ( USB_HID_Short_Item_t ) - HID_REPORT_ITEM_LOCATION_SIZE ))

swlock_t hidStaticVarLock = SWLOCK_INITIAL_VALUE;

/*
 * Each element in s_hidChangePending corresponds to an element in hidReports.
 */

static unsigned s_hidChangePending[ HID_REPORT_COUNT ];
static unsigned char s_hidReportDescriptor[ HID_REPORT_DESCRIPTOR_MAX_LENGTH ];
static size_t s_hidReportDescriptorLength;
static unsigned s_hidReportDescriptorPrepared;

static unsigned s_hidCurrentPeriod[ HID_REPORT_COUNT ];
static unsigned s_hidIdleActive[ HID_REPORT_COUNT ];
static unsigned s_hidNextReportTime[ HID_REPORT_COUNT ];
static unsigned s_hidReportTime[ HID_REPORT_COUNT ];

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

unsigned hidIsReportIdInUse ( void ) {
    return !hidIsReportIdValid(0U);
}

void hidCalcNextReportTime( const unsigned id )
{
    swlock_acquire(&hidStaticVarLock);
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx ) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            s_hidNextReportTime[ idx ] = s_hidReportTime[ idx ] + s_hidCurrentPeriod[ idx ];
        }
    }
    swlock_release(&hidStaticVarLock);
}

void hidCaptureReportTime( const unsigned id, const unsigned time )
{
    swlock_acquire(&hidStaticVarLock);
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx ) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            s_hidReportTime[ idx ] = time;
        }
    }
    swlock_release(&hidStaticVarLock);
}

void hidClearChangePending( const unsigned id )
{
    swlock_acquire(&hidStaticVarLock);
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx) {
        if(( id == 0U ) || ( id == hidGetElementReportId( hidReports[ idx ]->location ))) {
            s_hidChangePending[ idx ] = 0U;
            break;
        }
    }
    swlock_release(&hidStaticVarLock);
}

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

unsigned hidGetNextReportTime( const unsigned id ) {
    swlock_acquire(&hidStaticVarLock);
    unsigned retVal = 0U;

    for ( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx ) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            retVal = s_hidNextReportTime[ idx ];
        }
     }
    swlock_release(&hidStaticVarLock);
    return retVal;
}

unsigned char* hidGetReportDescriptor( void )
{
    unsigned char* retVal = NULL;
    swlock_acquire(&hidStaticVarLock);

    if( s_hidReportDescriptorPrepared ) {
        retVal = s_hidReportDescriptor;
    }

    swlock_release(&hidStaticVarLock);
    return retVal;
}

size_t hidGetReportDescriptorLength( void )
{
    swlock_acquire(&hidStaticVarLock);
    size_t retVal = ( s_hidReportDescriptorPrepared ) ? s_hidReportDescriptorLength : 0U;
    swlock_release(&hidStaticVarLock);
    return retVal;
}

unsigned hidGetReportIdLimit ( void ) {
    unsigned retVal = 0U;

    swlock_acquire(&hidStaticVarLock);
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx ) {
        unsigned reportId = hidGetElementReportId( hidReports[ idx ]->location );
        if( reportId >= retVal ) {
            retVal = reportId + 1;
        }
    }
    swlock_release(&hidStaticVarLock);
    return retVal;
}

unsigned hidGetNextValidReportId ( unsigned idPrev ) {
    size_t retIndex = 0;
    swlock_acquire(&hidStaticVarLock);
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx ) {
        unsigned reportId = hidGetElementReportId( hidReports[ idx ]->location );
        if( reportId == idPrev ) {
            retIndex = (idx + 1) % HID_REPORT_COUNT;
            break;
        }
    }

    unsigned retVal = hidGetElementReportId( hidReports[ retIndex ]->location );
    swlock_release(&hidStaticVarLock);
    return retVal;
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
        swlock_acquire(&hidStaticVarLock);
        USB_HID_Report_Element_t element = *hidConfigurableElements[ elementIdx ];
        swlock_release(&hidStaticVarLock);

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
    swlock_acquire(&hidStaticVarLock);
    size_t retVal = 0U;
    if( s_hidReportDescriptorPrepared ) {
        for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx ) {
            if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
                retVal = hidGetElementReportLength( hidReports[ idx ]->location );
            }
        }
    }
    swlock_release(&hidStaticVarLock);
    return retVal;
}

unsigned hidGetReportPeriod( const unsigned id )
{
    swlock_acquire(&hidStaticVarLock);
    unsigned retVal = 0U;
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            retVal = s_hidCurrentPeriod[ idx ];
            break;
        }
    }
    swlock_release(&hidStaticVarLock);
    return retVal;
}

unsigned hidGetReportTime( const unsigned id )
{
    swlock_acquire(&hidStaticVarLock);
    unsigned retVal = 0U;

    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx ) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            retVal = s_hidReportTime[ idx ];
        }
    }

    swlock_release(&hidStaticVarLock);
    return retVal;
}

static unsigned hidGetUsagePage( const unsigned id )
{
    unsigned retVal = 0U;
    swlock_acquire(&hidStaticVarLock);

    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            retVal = hidReports[ idx ]->item.data[ 0 ];
            break;
        }
    }

    swlock_release(&hidStaticVarLock);
    return retVal;
}

unsigned hidIsChangePending( const unsigned id )
{
    unsigned retVal = 0U;
    swlock_acquire(&hidStaticVarLock);

    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            retVal  = ( s_hidChangePending[ idx ] != 0U );
            break;
        }
    }

  swlock_release(&hidStaticVarLock);
  return retVal;
}

unsigned hidIsIdleActive( const unsigned id )
{
    unsigned retVal = 0U;

    swlock_acquire(&hidStaticVarLock);
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            retVal  = ( s_hidIdleActive[ idx ] != 0U );
            break;
        }
    }
    swlock_release(&hidStaticVarLock);
    return retVal;
}

unsigned hidIsReportDescriptorPrepared( void )
{
    swlock_acquire(&hidStaticVarLock);
    unsigned retVal = s_hidReportDescriptorPrepared;
    swlock_release(&hidStaticVarLock);
    return retVal;
}

unsigned hidIsReportIdValid ( unsigned id ) {
    size_t retVal = 0;

    swlock_acquire(&hidStaticVarLock);
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx ) {
        unsigned reportId = hidGetElementReportId( hidReports[ idx ]->location );
        if( reportId == id ) {
            retVal = 1;
            break;
        }
    }

    swlock_release(&hidStaticVarLock);
    return retVal;
}

void hidPrepareReportDescriptor( void )
{
    swlock_acquire(&hidStaticVarLock);
    if( !s_hidReportDescriptorPrepared ) {
        s_hidReportDescriptorLength = 0U;
        unsigned char* ptr = s_hidReportDescriptor;

        for( size_t idx = 0U; idx < HID_REPORT_DESCRIPTOR_ITEM_COUNT; ++idx ) {
            s_hidReportDescriptorLength += hidTranslateItem( hidReportDescriptorItems[ idx ], &ptr );
        }

        s_hidReportDescriptorPrepared = 1U;
    }
    swlock_release(&hidStaticVarLock);
}

void hidReportInit( void )
{
    swlock_acquire(&hidStaticVarLock);
    for( unsigned idx = 0U; idx < HID_REPORT_COUNT; ++idx ) {
        s_hidCurrentPeriod[ idx ] = ENDPOINT_INT_INTERVAL_IN_HID * MS_IN_TICKS * HID_REPORT_COUNT;
    }
    memset( s_hidIdleActive, 0, sizeof( s_hidIdleActive ) );
    memset( s_hidChangePending, 0, sizeof( s_hidChangePending ) );
    swlock_release(&hidStaticVarLock);
}

void hidResetReportDescriptor( void )
{
    swlock_acquire(&hidStaticVarLock);
    s_hidReportDescriptorPrepared = 0U;
    swlock_release(&hidStaticVarLock);
}

void hidSetChangePending( const unsigned id )
{
    swlock_acquire(&hidStaticVarLock);
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            s_hidChangePending[ idx ] = 1U;
            break;
        }
    }
    swlock_release(&hidStaticVarLock);
}

void hidSetIdle( const unsigned id, const unsigned state )
{
    swlock_acquire(&hidStaticVarLock);
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            s_hidIdleActive[ idx ] = ( state != 0U );
            break;
        }
    }
    swlock_release(&hidStaticVarLock);
}

void hidSetNextReportTime( const unsigned id, const unsigned time )
{
    swlock_acquire(&hidStaticVarLock);
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx ) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            s_hidNextReportTime[ idx ] = time;
        }
    }
    swlock_release(&hidStaticVarLock);
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

    if( !s_hidReportDescriptorPrepared ) {
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
                swlock_acquire(&hidStaticVarLock);
                USB_HID_Report_Element_t element = *hidConfigurableElements[ elementIdx ];
                swlock_release(&hidStaticVarLock);

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

                            swlock_acquire(&hidStaticVarLock);
                            *hidConfigurableElements[ elementIdx ] = element;
                            swlock_release(&hidStaticVarLock);
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

void hidSetReportPeriod( const unsigned id, const unsigned period )
{
    swlock_acquire(&hidStaticVarLock);
    for( size_t idx = 0U; idx < HID_REPORT_COUNT; ++idx) {
        if( id == hidGetElementReportId( hidReports[ idx ]->location )) {
            s_hidCurrentPeriod[ idx ] = period;
            break;
        }
    }
    swlock_release(&hidStaticVarLock);
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

// hid_report_descriptor.h validation functions for development purposes

/**
 * @brief Internal HID Report Descriptor validation state
 */
struct HID_validation_info {
    int collectionOpenedCount; //!< Current count of open collections (to track that they are all closed)
    int reportCount; //!< Current count of defined reports (to count them)

    int currentReportIdx; //!< Index of current report in hidReports array
    int currentConfigurableElementIdx; //!< Index of current configurable element in hidConfigurableElements array

    unsigned char reportIds[HID_REPORT_COUNT]; // Array of report IDs (for general validation & duplication detection)
    unsigned reportUsagePage[HID_REPORT_COUNT]; // Array of the usage page for each report (for general validation)

    unsigned current_bit_size;  // State tracker for the current set report bit width (todo: should technically be a stack)
    unsigned current_bit_count;  // State tracker for the current set report count (todo: should technically be a stack)
    unsigned current_bit_offset; // Current bit offset into this report (for location validation)
};

/**
 * @brief Validation step for hidReportValidate, checking the info struct to ensure correctness of Report IDs
 *
 * @param info The info struct that has been built by hidReportValidate to check
 * @return unsigned HID_STATUS value
 */
static unsigned hidReportValidateInfoStructReportIDs( struct HID_validation_info *info ) {
    if ( info->reportCount != HID_REPORT_COUNT) {
        if ( !( info->reportCount == 0 && HID_REPORT_COUNT == 1 ) ) {
            // (Only if report IDs are being used)
            printf("Error: The number of actual reports does not match HID_REPORT_COUNT.\n");
            return HID_STATUS_BAD_REPORT_DESCRIPTOR;
        }
    }
    for ( size_t idx1 = 0; idx1 < HID_REPORT_COUNT; ++idx1 ) {
        for ( size_t idx2 = idx1 + 1; idx2 < HID_REPORT_COUNT; ++idx2 ) {
            if ( info->reportIds[idx1] == info->reportIds[idx2] ) {
                printf("Error: Duplicate report ID 0x%02x.\n", info->reportIds[idx1]);
                return HID_STATUS_BAD_REPORT_DESCRIPTOR;
            }
        }
    }
    for ( size_t idx = 0; idx < HID_REPORT_COUNT; ++idx ) {
        if ( info->reportIds[idx] != hidGetElementReportId( hidReports[idx]->location ) ) {
            printf("Error: Report ID in descriptor does not match report ID in hidReports.\n");
            return HID_STATUS_BAD_REPORT_DESCRIPTOR;
        }
        if ( info->reportCount && info->reportIds[idx] == 0 ) {
            printf("Error: Report ID 0 is invalid.\n");
            return HID_STATUS_BAD_REPORT_DESCRIPTOR;
        }
    }
    return HID_STATUS_GOOD;
}

/**
 * @brief Validation step for hidReportValidate, checking reports are the correct length specified in their location field
 *
 * @param info The info struct that has been built by hidReportValidate to check
 * @return unsigned HID_STATUS value
 */
static unsigned hidReportValidateInfoStructReportLength( struct HID_validation_info *info ) {
    if ( info->current_bit_offset % 8 ) {
        printf("Error: HID Report not byte aligned (%d bits).\n", info->current_bit_offset);
        return HID_STATUS_BAD_REPORT_DESCRIPTOR;
    }
    if ( ( info->current_bit_offset / 8 ) != hidGetElementReportLength( hidReports[info->currentReportIdx]->location ) ) {
        printf("Error: Actual report length does not match value in location field %d != %d.\n",
            ( info->current_bit_offset / 8 ),
            hidGetElementReportLength( hidReports[info->currentReportIdx]->location ));
        return HID_STATUS_BAD_REPORT_DESCRIPTOR;
    }
    return HID_STATUS_GOOD;
}

/**
 * @brief Validation step for hidReportValidate, collections are correctly opened and closed
 *
 * @param info The info struct that has been built by hidReportValidate to check
 * @return unsigned HID_STATUS value
 */
static unsigned hidReportValidateInfoStructCollections( struct HID_validation_info *info ) {
    if ( info->collectionOpenedCount ) {
        printf("Error: Collections not equally opened and closed.\n");
        return HID_STATUS_BAD_REPORT_DESCRIPTOR;
    }
    return HID_STATUS_GOOD;
}

/**
 * @brief Validation step for hidReportValidate, High level - Checks the summarised information in the info struct by calling
 * the subroutines for checking.
 *
 * @param info The info struct that has been built by hidReportValidate to check
 * @return unsigned HID_STATUS value
 */
static unsigned hidReportValidateInfoStruct( struct HID_validation_info *info ) {
    unsigned status = hidReportValidateInfoStructCollections( info );
    if( status == HID_STATUS_GOOD ) {
        status = hidReportValidateInfoStructReportIDs( info );
    }
    if( status == HID_STATUS_GOOD ) {
        status = hidReportValidateInfoStructReportLength( info );
    }
    return status;
}

/**
 * @brief Preparation step for hidReportValidate, Adds a report ID field into the information struct for validation
 *
 * @param info The info struct being built by hidReportValidate
 * @param item The ReportId item being added
 * @return unsigned HID_STATUS value
 */
static unsigned hidReportValidateAddReportId( struct HID_validation_info *info, const USB_HID_Short_Item_t *item ) {
    if ( info->reportCount == 0 ) {
        if ( info->current_bit_offset ) {
            printf("Error: Some elements not associated with report ID.\n");
            return HID_STATUS_BAD_REPORT_DESCRIPTOR;
        }
        info->reportUsagePage[0] = 0;
    } else {
        unsigned status = hidReportValidateInfoStructReportLength( info );
        if ( status ) {
            return status;
        }
    }

    if ( hidGetItemSize(item->header) != 1 ) {
        printf("Error: ReportId field has invalid length %d (expected 1)\n", hidGetItemSize(item->header));
        return HID_STATUS_BAD_REPORT_DESCRIPTOR;
    }

    info->reportIds[info->reportCount] = item->data[0];
    info->currentReportIdx = info->reportCount;
    info->reportCount += 1;
    info->current_bit_offset = 0;
    if ( info->reportCount > HID_REPORT_COUNT ) {
        printf("Error: HID_REPORT_COUNT does not match number of report IDs in descriptor.\n");
        return HID_STATUS_BAD_REPORT_DESCRIPTOR;
    }

    return HID_STATUS_GOOD;
}

/**
 * @brief Preparation step for hidReportValidate, Adds a Usage Page field into the information struct for validation
 *
 * @param info The info struct being built by hidReportValidate
 * @param item The UsagePage item being added
 * @return unsigned HID_STATUS value
 */
static unsigned hidReportValidateAddUsagePageItem( struct HID_validation_info *info, const USB_HID_Short_Item_t *item ) {

    if ( info->collectionOpenedCount == 0 ) {
        return HID_STATUS_GOOD;
    }
    if ( info->reportUsagePage[info->currentReportIdx] ) {
        printf("Error: Multiple usage pages per report ID not supported by this implementation.\n");
        return HID_STATUS_BAD_REPORT_DESCRIPTOR;
    }

    switch (hidGetItemSize(item->header)){
    case 1:
        info->reportUsagePage[info->currentReportIdx] = item->data[0];
        break;
    case 2:
        info->reportUsagePage[info->currentReportIdx] = ((unsigned) item->data[1] << 8) + item->data[0];
        break;
    default:
        printf("Error: Invalid size for UsagePage report descriptor item.\n");
        return HID_STATUS_BAD_REPORT_DESCRIPTOR;
    }

    return HID_STATUS_GOOD;
}

/**
 * @brief Preparation step for hidReportValidate, Adds a Usage field into the information struct for validation
 *
 * @param info The info struct being built by hidReportValidate
 * @param item The Usage item being added
 * @return unsigned HID_STATUS value
 */
static unsigned hidReportValidateAddUsageItem( struct HID_validation_info *info, const USB_HID_Short_Item_t *item) {
    if ( ( info->currentConfigurableElementIdx < HID_CONFIGURABLE_ELEMENT_COUNT ) &&
         ( &(hidConfigurableElements[info->currentConfigurableElementIdx]->item) == item ) ) {

        USB_HID_Report_Element_t *element = hidConfigurableElements[info->currentConfigurableElementIdx];
        unsigned bBit = hidGetElementBitLocation( element->location );
        unsigned bByte = hidGetElementByteLocation( element->location );
        unsigned bReportId = hidGetElementReportId( element->location );

        if ( bBit != ( info->current_bit_offset % 8 ) || bByte != ( info->current_bit_offset / 8 ) ) {
            printf("Error: Locator bit/byte setting incorrect for configurable element index %d.\n", info->currentConfigurableElementIdx);
            return HID_STATUS_BAD_REPORT_DESCRIPTOR;
        }
        if ( bReportId != info->reportIds[info->currentReportIdx] ) {
            printf("Error: Locator report ID setting incorrect for configurable element index %d.\n", info->currentConfigurableElementIdx);
            return HID_STATUS_BAD_REPORT_DESCRIPTOR;
        }

        info->currentConfigurableElementIdx += 1;
    }
    return HID_STATUS_GOOD;
}

unsigned hidReportValidate( void )
{
    struct HID_validation_info info = {};
    unsigned status = HID_STATUS_GOOD;

    // Fill in the validation info struct by iterating through the hid report items
    for ( size_t idx = 0; idx < HID_REPORT_DESCRIPTOR_ITEM_COUNT; ++idx ) {
        const USB_HID_Short_Item_t *item = hidReportDescriptorItems[idx];
        unsigned bTag  = hidGetItemTag ( item->header );
        unsigned bType = hidGetItemType( item->header );

        if ( bTag == HID_REPORT_ITEM_TAG_COLLECTION && bType == HID_REPORT_ITEM_TYPE_MAIN ) {
            info.collectionOpenedCount += 1;
        }
        else if ( bTag == HID_REPORT_ITEM_TAG_END_COLLECTION && bType == HID_REPORT_ITEM_TYPE_MAIN ) {
            info.collectionOpenedCount -= 1;
            if ( info.collectionOpenedCount < 0 ) {
                printf("Error: Collection closed while there is no collection open.\n");
                status = HID_STATUS_BAD_REPORT_DESCRIPTOR;
            }
        }
        else if ( bTag == HID_REPORT_ITEM_TAG_INPUT && bType == HID_REPORT_ITEM_TYPE_MAIN ) {
            info.current_bit_offset += (info.current_bit_size * info.current_bit_count);
        }
        else if ( bTag == HID_REPORT_ITEM_TAG_REPORT_SIZE && bType == HID_REPORT_ITEM_TYPE_GLOBAL ) {
            info.current_bit_size = item->data[0];
        }
        else if ( bTag == HID_REPORT_ITEM_TAG_REPORT_COUNT && bType == HID_REPORT_ITEM_TYPE_GLOBAL ) {
            info.current_bit_count = item->data[0];
        }
        else if ( bTag == HID_REPORT_ITEM_TAG_REPORT_ID && bType == HID_REPORT_ITEM_TYPE_GLOBAL ) {
            status = hidReportValidateAddReportId( &info, item );
        }
        else if ( bTag == HID_REPORT_ITEM_TAG_USAGE_PAGE && bType == HID_REPORT_ITEM_TYPE_GLOBAL ) {
            status = hidReportValidateAddUsagePageItem( &info, item );
        }
        else if ( bTag == HID_REPORT_ITEM_TAG_USAGE && bType == HID_REPORT_ITEM_TYPE_LOCAL ) {
            status = hidReportValidateAddUsageItem( &info, item );
        }

        if ( status ) {
            break;
        }
    }

    if(status) {
        return status;
    } else {
        return hidReportValidateInfoStruct( &info );
    }
}

#endif // ( 0 < HID_CONTROLS )

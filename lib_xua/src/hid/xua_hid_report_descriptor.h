// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) Report descriptor
 *
 * This file defines the structure and default content of the HID Report descriptor.
 * Document section numbers refer to the HID Device Class Definition, version 1.11.
 */

#ifndef _HID_REPORT_DESCRIPTOR_
#define _HID_REPORT_DESCRIPTOR_

#define HID_REPORT_ITEM_HDR_SIZE_MASK   ( 0x03 )
#define HID_REPORT_ITEM_HDR_SIZE_SHIFT  ( 0 )

#define HID_REPORT_ITEM_HDR_TAG_MASK    ( 0xF0 )
#define HID_REPORT_ITEM_HDR_TAG_SHIFT   ( 4 )

#define HID_REPORT_ITEM_HDR_TYPE_MASK   ( 0x0C )
#define HID_REPORT_ITEM_HDR_TYPE_SHIFT  ( 2 )

#define HID_REPORT_ITEM_MAX_SIZE        ( 2 )

#define HID_REPORT_ITEM_USAGE_TAG       ( 0 )
#define HID_REPORT_ITEM_USAGE_TYPE      ( 2 )

#define HID_STATUS_GOOD                 ( 0 )
#define HID_STATUS_BAD_HEADER           ( 1 )
#define HID_STATUS_BAD_LOCATION         ( 2 )

/**
 * @brief USB HID Report Descriptor. Short Item
 *
 * @note
 * To reduce memory use, this type does not support Short Items with 4 data bytes.
 * See section 6.2.2.2
 *
 * Elements:
 *
 *   header   - the item prefix containing the size, type and tag fields (see 6.2.2.2)
 *              Format (bit range): bSize (0:1), bType (2:3), bTag (4:7)
 *   data     - a two byte array for holding the item's data
 *              The bSize field indicates which data bytes are in use
 *   location - a non-standard extension locating the item within the HID Report
 *              Format (bit range): iByte (0:3), iBit (4:6), Reserved (7)
 */
typedef struct
{
    unsigned char header;
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ];
    unsigned char location;
} USB_HID_Short_Item_t;

/**
 * @brief Get the HID Report descriptor
 *
 * This function returns a pointer to the USB HID Report descriptor.
 * It returns NULL if the Report descriptor has not been initialised,
 *   i.e., no one has called \c hidInitReportDescriptor().
 *
 * @return A pointer to a list of unsigned char containing the Report descriptor
 */
unsigned char* hidGetReportDescriptor( void );

/**
 * @brief Prepare the USB HID Report descriptor
 *
 * After preparation, \c hidGetReportDescriptor() returns a list suitablefor transmission over USB.
 *
 * Call this function after altering one or more Report Items using \c hidSetReportItem().
 */
void hidPrepareReportDescriptor( void );

/**
 * @brief Modify a HID Report descriptor item
 *
 * @warning This function does not check that the length of the \a data array matches the value of
 *          the bSize field in the \a header.  For safe operation use a \a data array of at least
 *          \c HID_REPORT_ITEM_MAX_SIZE bytes in length.
 *
 * Parameters:
 *
 *  @param[in] byte     The byte position of the control within the HID Report
 *  @param[in] bit      The bit position of the control within the \a byte
 *  @param[in] header   The LSB of the Item containing the bSize, bType and bTag fields (see 6.2.2.2)
 *  @param[in] data     An array containing data bytes or NULL for an Item with no data
 *
 * @return A status value
 * @retval \c HID_STATUS_GOOD           Item successfully updated
 * @retval \c HID_STATUS_BAD_HEADER     The Item header specified a data size greater than 2 or
 *                                      a Tag or Type inconsistent with a Usage Item
 * @retval \c HID_STATUS_BAD_LOCATION   The \a bit or \a byte arguments specify a location outside
 *                                      of the HID Report
 */
unsigned hidSetReportItem( const unsigned byte, const unsigned bit, const unsigned char header, const unsigned char data[] );

#endif // _HID_REPORT_DESCRIPTOR_

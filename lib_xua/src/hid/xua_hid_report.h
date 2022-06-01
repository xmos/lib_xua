// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) Report descriptor
 *
 * This file defines the structure of the HID Report descriptor and declares
 *   functions for manipulating it.
 * Because the Report descriptor defines the length of the HID Report, this file
 *   declares a function for obtaining the Report length as well.
 * The using application has the responsibility to define the report descriptor
 *   structure and default contents in their hid_report_descriptor.h file.
 * Document section numbers refer to the HID Device Class Definition, version 1.11.
 */

#ifndef _XUA_HID_REPORT_
#define _XUA_HID_REPORT_

#include <stddef.h>

#include "xua_hid_report_descriptor_constants.h"

#define HID_REPORT_ITEM_HDR_SIZE_MASK       ( 0x03 )
#define HID_REPORT_ITEM_HDR_SIZE_SHIFT      ( 0U )

#define HID_REPORT_ITEM_HDR_TAG_MASK        ( 0xF0 )
#define HID_REPORT_ITEM_HDR_TAG_SHIFT       ( 4U )

#define HID_REPORT_ITEM_HDR_TYPE_MASK       ( 0x0C )
#define HID_REPORT_ITEM_HDR_TYPE_SHIFT      ( 2U )

#define HID_REPORT_ELEMENT_LOC_BIT_MASK     ( 0x0070 )
#define HID_REPORT_ELEMENT_LOC_BIT_SHIFT    ( 4U )

#define HID_REPORT_ELEMENT_LOC_BYTE_MASK    ( 0x000F )
#define HID_REPORT_ELEMENT_LOC_BYTE_SHIFT   ( 0U )

#define HID_REPORT_ELEMENT_LOC_ID_MASK      ( 0xF000 )
#define HID_REPORT_ELEMENT_LOC_ID_SHIFT     ( 12U )

#define HID_REPORT_ELEMENT_LOC_LEN_MASK     ( 0x0F00 )
#define HID_REPORT_ELEMENT_LOC_LEN_SHIFT    ( 8U )

#define HID_REPORT_ITEM_MAX_SIZE            ( 2U )

#define HID_REPORT_ITEM_USAGE_TAG           ( 0U )
#define HID_REPORT_ITEM_USAGE_TYPE          ( 2U )

/**
 * @brief Helper macro to configure the location field of USB_HID_Report_Element_t.
 *
 * @param id   The report ID that this element is within.
 * @param len  (only relevant for the usage_page elements in hidReports) The length
 *             of the report under this report ID.
 * @param byte The byte location of this element in the report.
 * @param bit  The bit location (within the byte) of this element in the report.
 */
#define HID_REPORT_SET_LOC(id, len, byte, bit) (\
    ((   id << HID_REPORT_ELEMENT_LOC_ID_SHIFT   ) & HID_REPORT_ELEMENT_LOC_ID_MASK   ) | \
    ((  len << HID_REPORT_ELEMENT_LOC_LEN_SHIFT  ) & HID_REPORT_ELEMENT_LOC_LEN_MASK  ) | \
    (( byte << HID_REPORT_ELEMENT_LOC_BYTE_SHIFT ) & HID_REPORT_ELEMENT_LOC_BYTE_MASK ) | \
    ((  bit << HID_REPORT_ELEMENT_LOC_BIT_SHIFT  ) & HID_REPORT_ELEMENT_LOC_BIT_MASK  ))

/**
 * @brief Helper macro to configure the header field of USB_HID_Short_Item_t
 *
 * @param size The size of the report descriptor item (valid values: 0, 1, 2)
 * @param type The type of the report descriptor item
 * @param tag  The tag
 */
#define HID_REPORT_SET_HEADER(size, type, tag) (\
    (( size << HID_REPORT_ITEM_HDR_SIZE_SHIFT) & HID_REPORT_ITEM_HDR_SIZE_MASK ) |\
    (( type << HID_REPORT_ITEM_HDR_TYPE_SHIFT) & HID_REPORT_ITEM_HDR_TYPE_MASK ) |\
    (( tag  << HID_REPORT_ITEM_HDR_TAG_SHIFT ) & HID_REPORT_ITEM_HDR_TAG_MASK ) )

#define HID_STATUS_GOOD                  ( 0U )
#define HID_STATUS_BAD_HEADER            ( 1U )
#define HID_STATUS_BAD_ID                ( 2U )
#define HID_STATUS_BAD_LOCATION          ( 3U )
#define HID_STATUS_BAD_PAGE              ( 4U )
#define HID_STATUS_IN_USE                ( 5U )
#define HID_STATUS_BAD_REPORT_DESCRIPTOR ( 6U )

#define MS_IN_TICKS 100000U

/**
 * @brief USB HID Report Descriptor Short Item
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
 *   id       - a non-standard extension identifying the HID Report ID associated with
 *              the item (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *   location - a non-standard extension locating the item within the HID Report
 *              Format (bit range): iByte (0:3), iBit (4:6), Reserved (7)
 */
typedef struct
{
    unsigned char header;
    unsigned char data[ HID_REPORT_ITEM_MAX_SIZE ];
} USB_HID_Short_Item_t;

typedef struct
{
    USB_HID_Short_Item_t item;
    unsigned short location;
} USB_HID_Report_Element_t;

/**
 *  \brief Calculate the next time to respond with a HID Report.
 *
 *  If the USB Host has previously sent a valid HID Set_Idle request with
 *    a duration of zero or greater than the default reporting interval,
 *    the device sends HID Reports periodically or when the value of the
 *    payload has changed.
 *
 *  This function calculates the time for sending the next periodic
 *    HID Report.
 *
 * Parameters:
 *
 *  @param[in]  id  The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                  A value of zero means the application does not use Report IDs.
 */
void hidCalcNextReportTime( const unsigned id );

/**
 *  \brief Capture the time of sending the current HID Report.
 *
 *  If the USB Host has previously sent a valid HID Set_Idle request with
 *    a duration of zero or greater than the default reporting interval,
 *    the device sends HID Reports periodically or when the value of the
 *    payload has changed.
 *
 *  This function captures the time when the HID Report was sent so that
 *    a subsequent call to HidCalNextReportTime() can calculate the time
 *    to send the next periodic HID Report.
 *
 * Parameters:
 *
 *  @param[in]  id      The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                      A value of zero means the application does not use Report IDs.
 *
 *  @param[in]  time    The time when the HID Report for the given \a id was sent.
 */
void hidCaptureReportTime( const unsigned id, const unsigned time );

/**
 *  \brief Register that a previously changed HID Report data has been sent
 *         to the USB Host.
 *
 *  HID processing maintains a list of HID Reports with changed data not yet
 *    reported to the USB Host.
 *
 *  Applications that have only one HID Report may or may not use a Report ID.
 *  Applications that have more than one HID Report must use Report IDs.
 *
 *  For applications that do not use Report IDs, the list contains one element.
 *  That element tracks whether or not an unreported change has occurred in the
 *    HID data.
 *  For applications that use Report IDs, the list contains one element per
 *    Report ID.
 *  Each element tracks unreported changes for the corresponding Report ID.
 *
 *  Calling this function for a given Report ID indicates that the changed
 *    HID data has been reported to the USB Host.
 *
 *  \warning This function will fail silently if given an id that is not
 *    either the value zero (in the case that Report IDs are not in use),
 *    or a Report ID that is in use.
 *
 *  \param[in]  id  A HID Report ID.
 *                  Use zero if the application does not use Report IDs.
 */
void hidClearChangePending( const unsigned id );

/**
 * @brief Get the next valid report ID - iterator style.
 *
 * This function will loop around and start returning the first report ID again once it has
 * returned all valid report IDs.
 *
 * @param idPrev The previous returned id, or 0 if this is the first call
 * @return unsigned The next valid report ID.
 */
unsigned hidGetNextValidReportId ( unsigned idPrev );

/**
 * @brief Get the HID Report descriptor
 *
 * This function returns a pointer to the USB HID Report descriptor.
 * It returns NULL if the Report descriptor has not been prepared,
 *   i.e., no one has called \c hidPrepareReportDescriptor().
 *
 * @note An XC-callable version of this function has not been provided.
 *       XC requires explicit declaration of the kind of pointer returned,
 *       hence an XC implementation of the function.
 *
 * @return A pointer to a list of unsigned char containing the Report descriptor
 */
#if !defined(__XC__)
unsigned char* hidGetReportDescriptor( void );
#endif

/**
 * @brief Get the length of the HID Report descriptor
 *
 * This function returns the length of the USB HID Report descriptor.
 * It returns zero if the Report descriptor has not been prepared,
 *   i.e., no one has called \c hidPrepareReportDescriptor().
 *
 * @return The length of the Report descriptor in bytes
 */
size_t hidGetReportDescriptorLength( void );

/**
 *  \brief  Get the upper limit of HID Report identifiers
 *
 *  This function returns the upper limit of the HID Report identifiers.
 *  The upper limit has a value one greater than the maximum HID Report identifier.
 *  In the case that HID Report identifiers are not in use, this function returns the value 1.
 *
 *  \returns  The upper limit of HID Report identifiers
 */
unsigned hidGetReportIdLimit ( void );

/**
 * @brief Get a HID Report descriptor item
 *
 * Parameters:
 *
 *  @param[in]  id      The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2).
 *                      A value of zero means the application does not use Report IDs.
 *  @param[in]  byte    The byte position of the control within the HID Report
 *  @param[in]  bit     The bit position of the control within the \a byte
 *  @param[out] page    The USB HID Usage Page code for the Item (see 5.5)
 *  @param[out] header  The LSB of the Item containing the bSize, bType and bTag fields (see 6.2.2.2)
 *  @param[out] data    A two element array containing data bytes for the Item
 *
 * @return A status value
 * @retval \c HID_STATUS_GOOD           Item successfully returned
 * @retval \c HID_STATUS_BAD_ID         The \a id argument specifies a non-existant HID Report
 * @retval \c HID_STATUS_BAD_LOCATION   The \a bit or \a byte arguments specify a location outside
 *                                      of the HID Report
 */
#if defined(__XC__)
unsigned hidGetReportItem(
    const unsigned id,
    const unsigned byte,
    const unsigned bit,
    unsigned char* unsafe const page,
    unsigned char* unsafe const header,
    unsigned char* unsafe const data);
#else
unsigned hidGetReportItem(
    const unsigned id,
    const unsigned byte,
    const unsigned bit,
    unsigned char* const page,
    unsigned char* const header,
    unsigned char data[]);
#endif

/**
 * @brief Get the time to send the next HID Report for the given \a id
 *
 * Parameters:
 *
 *  @param[in]  id  The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                  A value of zero means the application does not use Report IDs.
 *
 *  @returns  The time at which to send the next HID Report for the given \a id
 */
unsigned hidGetNextReportTime( const unsigned id );

/**
 * @brief Get the length of the HID Report
 *
 * This function returns the length of the USB HID Report.
 * It returns zero if the Report descriptor has not been prepared,
 *   i.e., no one has called \c hidPrepareReportDescriptor(),
 *   or if the \a id argument specifies a non-existent HID Report
 *
 * Parameters:
 *
 *  @param[in]  id  The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                  A value of zero means the application does not use Report IDs.
 *
 * @return The length of the Report in bytes
 */
size_t hidGetReportLength( const unsigned id );

/**
 * @brief Get the HID Report period for the given \a id
 *
 * Parameters:
 *
 *  @param[in]  id  The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                  A value of zero means the application does not use Report IDs.
 *
 *  @returns  The period for the given HID Report \a id in units of ms.
 *            The value zero means the period is indefinite.
 */
unsigned hidGetReportPeriod( const unsigned id );

/**
 * @brief Get the HID Report time for the given \a id
 *
 * Parameters:
 *
 *  @param[in]  id      The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                      A value of zero means the application does not use Report IDs.
 *
 *  @returns  The time of the last call to \c hidCaptureReportTime()
 */
unsigned hidGetReportTime( const unsigned id );

/**
 *  \brief Indicate if a change to the HID Report data has been received.
 *
 *  HID processing maintains a list of HID Reports with changed data not yet
 *    reported to the USB Host.
 *
 *  Applications that have only one HID Report may or may not use a Report ID.
 *  Applications that have more than one HID Report must use Report IDs.
 *
 *  For applications that do not use Report IDs, the list contains one element.
 *  That element tracks whether or not an unreported change has occurred in the
 *    HID data.
 *  For applications that use Report IDs, the list contains one element per
 *    Report ID.
 *  Each element tracks unreported changes for the corresponding Report ID.
 *
 *  Calling this function with a given Report ID returns an indication of
 *   whether unreported HID data exists for that Report ID.
 *
 *  \warning This function will return zero if given an id that is not
 *    either the value zero (in the case that Report IDs are not in use),
 *    or a Report ID that is in use.
 *
 *  \param[in]  id  A HID Report ID.
 *                  Use zero if the application does not use Report IDs.
 *
 *  \returns  A Boolean indicating whether the given \a id has a changed
 *              HID Report not yet sent to the USB Host.
 *  \retval   True  The given \a id has changed HID Report data.
 *  \retval   False The given \a id does not have changed HID Report data.
 */
unsigned hidIsChangePending( const unsigned id );

/**
 * @brief Indicate if the HID report for the given \a id is idle
 *
 * Parameters:
 *
 *  @param[in]  id  The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *
 *  \returns  A Boolean indicating whether the HID Report for the given \a id is idle.
 *  \retval   True  The HID Report is idle.
 *  \retval   False The HID Report is not idle.
 */
unsigned hidIsIdleActive( const unsigned id );

/**
 * @brief Indicate if the HID Report descriptor has been prepared
 *
 *  \returns  A Boolean indicating whether the HID Report descriptor has been prepared.
 *  \retval   True  The HID Report descriptor has been prepared.
 *  \retval   False The HID Report descriptor has not been prepared.
 */
 unsigned hidIsReportDescriptorPrepared( void );

/**
 * @brief Does the application use Report IDs?
 *
 * If the application is not using Report IDs, then the id value that is passed around
 * everywhere can just be zero. Otherwise zero is an invalid ID.
 *
 * @return Boolean
 * @retval 1 Report IDs are in use
 * @retval 0 Report IDs are not in use
 */
unsigned hidIsReportIdInUse ( void );

/**
 * @brief Is the provided report ID valid for passing to other functions.
 *
 * e.g If Report IDs are not in use, then only 0 will return true.
 * e.g If Report IDs are in use, then 0 will return false and the report IDs that
 *     are in use will return true when passed to this function.
 *
 * @param id  The ID to check
 * @return boolean
 * @retval 0 The report ID is not valid, other functions may fail silently
 * @retval 1 The report ID is valid and can be used as the argument to other functions
 */
unsigned hidIsReportIdValid ( unsigned id );

/**
 * @brief Prepare the USB HID Report descriptor
 *
 * After preparation, \c hidGetReportDescriptor() returns a list suitable for transmission over USB.
 * Call this function after altering one or more Report Items using \c hidSetReportItem().
 */
void hidPrepareReportDescriptor( void );

/**
 * @brief Initialise the USB HID Report functionality
 *
 * Call this function before using any other functions in this API.
 */
void hidReportInit( void );

/**
 * @brief Reset the USB HID Report descriptor
 *
 * After reset, \c hidGetReportDescriptor() returns NULL until a subsequent call to
 *   \c hidPrepareReportDescriptor() occurs.
 * Call this function before altering one or more Report Items using \c hidSetReportItem().
 */
void hidResetReportDescriptor( void );

/**
 *  \brief Register that a change to the HID Report data has been received.
 *
 *  HID processing maintains a list of HID Reports with changed data not yet
 *    reported to the USB Host.
 *
 *  Applications that have only one HID Report may or may not use a Report ID.
 *  Applications that have more than one HID Report must use Report IDs.
 *
 *  For applications that do not use Report IDs, the list contains one element.
 *  That element tracks whether or not an unreported change has occurred in the
 *    HID data.
 *  For applications that use Report IDs, the list contains one element per
 *    Report ID.
 *  Each element tracks unreported changes for the corresponding Report ID.
 *
 *  Calling this function with a given Report ID indicates that the HID data
 *    for that Report ID has changed and has not yet been reported to the USB
 *    Host.
 *
 *  \warning This function will fail silently if given an id that is not
 *    either the value zero (in the case that Report IDs are not in use),
 *    or a Report ID that is in use.
 *
 *  \param[in]  id  A HID Report ID.
 *                  Use zero if the application does not use Report IDs.
 */
void hidSetChangePending( const unsigned id );

/**
 * @brief Set the HID Report Idle state for the given \a id
 *
 * Parameters:
 *
 *  @param[in]  id      The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                      A value of zero means the application does not use Report IDs.
 *
 *  @param[in]  state   A Boolean indicating the Idle state
 *                      If true, the HID Report for the given \a id is Idle, otherwise it
 *                        is not Idle.
 */
void hidSetIdle( const unsigned id, const unsigned state );

/**
 * @brief Set the time to send the HID Report for the given \a id
 *
 * Parameters:
 *
 *  @param[in]  id      The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                      A value of zero means the application does not use Report IDs.
 *
 *  @param[in]  time    The time to send the HID Report for the given \a id.
 */
void hidSetNextReportTime( const unsigned id, const unsigned time );

/**
 * @brief Modify a HID Report descriptor item
 *
 * @warning This function does not check that the length of the \a data array matches the value of
 *          the bSize field in the \a header.  For safe operation use a \a data array of at least
 *          \c HID_REPORT_ITEM_MAX_SIZE bytes in length.
 *
 * Parameters:
 *
 *  @param[in]  id      The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                      A value of zero means the application does not use Report IDs.
 *  @param[in] byte     The byte position of the control within the HID Report
 *  @param[in] bit      The bit position of the control within the \a byte
 *  @param[in] page     The USB HID Usage Page code for the Item (see 5.5)
 *  @param[in] header   The LSB of the Item containing the bSize, bType and bTag fields (see 6.2.2.2)
 *  @param[in] data     An array containing data bytes or NULL for an Item with no data
 *
 * @return A status value
 * @retval \c HID_STATUS_GOOD           Item successfully updated
 * @retval \c HID_STATUS_BAD_HEADER     The Item header specified a data size greater than 2 or
 *                                      a Tag or Type inconsistent with a Usage Item
 * @retval \c HID_STATUS_BAD_ID         The \a id argument specifies a non-existent HID Report
 * @retval \c HID_STATUS_BAD_LOCATION   The \a bit or \a byte arguments specify a location outside
 *                                      of the HID Report
 * @retval \c HID_STATUS_BAD_PAGE       The \a byte argument specifies a location for controls from
 *                                      a Usage Page other than the one given by the \a page parameter
 * @retval \c HID_STATUS_IN_USE         The Report descriptor is in use
 */
unsigned hidSetReportItem(
    const unsigned id,
    const unsigned byte,
    const unsigned bit,
    const unsigned char page,
    const unsigned char header,
    const unsigned char data[]);

/**
 * @brief Set the HID Report period for the given \a id
 *
 * Parameters:
 *
 *  @param[in]  id      The identifier for the HID Report (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                      A value of zero means the application does not use Report IDs.
 *
 *  @param[in]  period  The period for sending the HID Report in units of ms.
 *                      This period must be a multiple of 4 ms.
 *                      Use zero to indicate an indefinite period.
 */
void hidSetReportPeriod( const unsigned id, const unsigned period );

/**
 * @brief Development function: Validate the contents of hid_report_descriptor.h for common errors, printing
 * error messages if any issues were found.
 *
 * This function is intended for use when developing the contents of hid_report_descriptor.h, which is static,
 * so shouldn't be required for use in a production application.
 *
 * @return Validation result
 * @retval HID_STATUS_GOOD                    The validation found no issues with the data structures defined
 *                                            in hid_report_descriptor.h
 * @retval HID_STATUS_BAD_REPORT_DESCRIPTOR   The validation encountered an issue with the data structures
 *                                            defined in hid_report_descriptor.h . More information is
 *                                            provided in the printed messages.
 */
unsigned hidReportValidate( void );

#endif // _XUA_HID_REPORT_

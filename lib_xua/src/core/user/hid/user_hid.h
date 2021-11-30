// Copyright 2013-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) API
 *
 * This file defines the Application Programming Interface (API) used to record HID
 *   events and retrieve a HID Report for sending to a host.
 * The using application has the responsibility to fulfill this API.
 * Document section numbers refer to the HID Device Class Definition, version 1.11.
 */

#ifndef __USER_HID_H__
#define __USER_HID_H__

#include <stddef.h>

/**
 *  \brief  HID event
 *
 *  This struct identifies:
 *  - the HID Report that reports an event, i.e., the ID,
 *  - the location within the HID Report for the event, i.e., the byte and bit, and
 *  - the value to report for that location (typically interpreted as a Boolean).
 *  It assumes only single bit flags within the HID Report.
 */
typedef struct hidEvent_t {
  unsigned bit;
  unsigned byte;
  unsigned id;
  unsigned value;
} hidEvent_t;

#define HID_MAX_DATA_BYTES 4

#if( 0 < HID_CONTROLS )

/**
 *  \brief  Get the data for the next HID Report
 *
 *  \note This function returns the HID data as a list of unsigned char because the
 *        \c XUD_SetReady_In() accepts data for transmission to the USB Host using
 *        this type.
 *
 *  \param[in]  id       The HID Report ID (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                       Set to zero if the application provides only one HID Report
 *                         which does not include a Report ID
 *  \param[out] hidData  The HID data
 *                       If using Report IDs, this function places the Report ID in
 *                         the first element; otherwise the first element holds the
 *                         first byte of HID event data.
 *
 *  \returns  The length of the HID Report in the \a hidData argument
 *  \retval   Zero means no new HID event data has been recorded for the given \a id
 */
size_t UserHIDGetData( const unsigned id, unsigned char hidData[ HID_MAX_DATA_BYTES ]);

/**
 *	\brief  Initialize HID processing
 */
void UserHIDInit( void );

/**
 *  \brief  Record that a HID event has occurred
 *
 *  \param[in]  hidEvent      A list of events which have occurred.
 *                            Each element specifies a HID Report ID, a bit and byte
 *                              within the HID Report and the value for it.
 *                            Set the Report ID to zero if not using Report IDs
 *                              (see 5.6, 6.2.2.7, 8.1 and 8.2).
 *  \param[in]  hidEventCnt   The length of the \a hidEvent list.
 *
 *  \returns  The index of the first unrecorded event in \a hidEvent
 *  \retval   Zero indicates no events were recorded
 *  \retval   \a hidEventCnt indicates all events were recorded
 */
size_t UserHIDRecordEvent( const hidEvent_t hidEvent[], const size_t hidEventCnt );

/**
 *  \brief  Indicate if a HID Report ID has new data to report
 *
 *  \param[in]  id  A HID Report ID (see 5.6, 6.2.2.7, 8.1 and 8.2).
 *                  A value of zero means the application does not use Report IDs.
 *
 *  \returns  A Boolean flag indicating the status of the operation
 *  \retval   False The given \a id either does not have new data to report or lies
 *              outside the range of supported HID Report identifiers
 *  \retval   True  The given \a id has new data to report to the USB Host
 */
unsigned UserHIDReportChanged( const unsigned id );

#endif /* ( 0 < HID_CONTROLS ) */
#endif /* __USER_HID_H__ */

// Copyright 2013-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief Human Interface Device (HID) API
 *
 * This file defines the Application Programming Interface (API) used to record HID
 * events and retrieve a HID Report for sending to a host.
 * The using application has the responsibility to fulfill this API.
 * Document section numbers refer to the HID Device Class Definition, version 1.11.
 */

#ifndef _USER_HID_H_
#define _USER_HID_H_

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

#define HID_MAX_DATA_BYTES ( 4 )
#define HID_EVENT_INVALID_ID ( 0x100 )

/**
 *  \brief  Get the data for the next HID Report
 *
 *  \param[in]  id       The HID Report ID (see 5.6, 6.2.2.7, 8.1 and 8.2)
 *                       Set to zero if the application provides only one HID Report
 *                       which does not include a Report ID
 *  \param[out] hidData  The HID data
 *                       If using Report IDs, this function places the Report ID in
 *                       the first element; otherwise the first element holds the
 *                       first byte of HID event data.
 *
 *  \returns  The length of the HID Report in the \a hidData argument
 *  \retval   Zero means no new HID event data has been recorded for the given \a id
 */
size_t UserHIDGetData( const unsigned id, unsigned char hidData[ HID_MAX_DATA_BYTES ]);

/**
 *  \brief  Initialize HID processing
 */
void UserHIDInit( void );

#endif /* _USER_HID_H_ */

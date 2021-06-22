// Copyright 2013-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __USER_HID_H__
#define __USER_HID_H__

/**
 *  \brief  HID event
 *
 *  This struct identifies the location within the HID Report for an event and
 *  The value to report for that location.
 *  It assumes only single bit flags within the HID Report.
 */
typedef struct hidEvent_t {
  unsigned bit;
  unsigned byte;
  unsigned value;
} hidEvent_t;

#define HID_MAX_DATA_BYTES 4

#if( 0 < HID_CONTROLS )

/**
 *  \brief  Get the data for the next HID report
 *
 *  \note This function returns the HID data as a list of unsigned char because the
 *        \c XUD_SetReady_In() accepts data for transmission to the USB Host using
 *        this type.
 *
 *  \param{out} hidData  The HID data
 */
void UserHIDGetData( unsigned char hidData[ HID_MAX_DATA_BYTES ]);

/**
 *	\brief  Initialize HID processing
 */
void UserHIDInit( void );

/**
 *  \brief  Record that a HID event has occurred
 *
 *  \param{in}  hidEvent      A list of events which have occurred.
 *                            Each element specifies a bit and byte in the HID Report and the value for it.
 *  \param{in}  hidEventCnt   The length of the \a hidEvent list.
 *
 *  \returns  A Boolean flag indicating the status of the operation
 *  \retval   False No recording of the event(s) occurred
 *  \retval   True  Recording of the event(s) occurred
 */
unsigned UserHIDRecordEvent( const hidEvent_t hidEvent[], const unsigned hidEventCnt );

#endif /* ( 0 < HID_CONTROLS ) */
#endif /* __USER_HID_H__ */

// Copyright 2013-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __USER_HID_H__
#define __USER_HID_H__

/**
 *  \brief  HID event identifiers
 *
 *  This enumeration defines a constant value for each HID event.
 *  It defines one value for each of the four GPI pins supported in the standard voice products.
 *  It defines a further 28 values for generic events.
 */
typedef enum hidEventId_t {
  HID_EVENT_ID_EVT0 = 0,
  HID_EVENT_ID_EVT1,
  HID_EVENT_ID_EVT2,
  HID_EVENT_ID_EVT3,
  HID_EVENT_ID_EVT4,
  HID_EVENT_ID_EVT5,
  HID_EVENT_ID_EVT6,
  HID_EVENT_ID_EVT7,
  HID_EVENT_ID_EVT8,
  HID_EVENT_ID_EVT9,
  HID_EVENT_ID_EVT10,
  HID_EVENT_ID_EVT11,
  HID_EVENT_ID_EVT12,
  HID_EVENT_ID_EVT13,
  HID_EVENT_ID_EVT14,
  HID_EVENT_ID_EVT15,
  HID_EVENT_ID_EVT16,
  HID_EVENT_ID_EVT17,
  HID_EVENT_ID_EVT18,
  HID_EVENT_ID_EVT19,
  HID_EVENT_ID_EVT20,
  HID_EVENT_ID_EVT21,
  HID_EVENT_ID_EVT22,
  HID_EVENT_ID_EVT23,
  HID_EVENT_ID_EVT24,
  HID_EVENT_ID_EVT25,
  HID_EVENT_ID_EVT26,
  HID_EVENT_ID_EVT27,
  HID_EVENT_ID_EVT28,
  HID_EVENT_ID_EVT29,
  HID_EVENT_ID_EVT30,
  HID_EVENT_ID_EVT31,
  HID_EVENT_ID_INVALID = 0xffffffff,
} hidEventId_t;

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
 *  \param{in}  hidEventId    The identifier of an event which has occurred.
 *                            Each event corresponds to bit in the HID Report:
 *                            Events 0- 7 to bits 0-7 of byte 0,
 *                            Events 8-15 to bits 0-7 of byte 1, etc.
 *  \param{in}  hidEventData  A Boolean indicating the state of the event:
 *                            Zero = deasserted,
 *                            Any other value = asserted.
 */
void UserHIDRecordEvent( const hidEventId_t hidEventId, const unsigned hidEventData );

#endif /* ( 0 < HID_CONTROLS ) */
#endif /* __USER_HID_H__ */

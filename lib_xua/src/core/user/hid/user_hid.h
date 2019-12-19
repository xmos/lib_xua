// Copyright (c) 2013-2019, XMOS Ltd, All rights reserved

#ifndef __USER_HID_H__
#define __USER_HID_H__

/* These enumerated constants relate to the HID report desc - do not mod */
typedef enum hidEventId_t {
  HID_EVENT_ID_GPI0 = 0,
  HID_EVENT_ID_GPI1,
  HID_EVENT_ID_GPI2,
  HID_EVENT_ID_GPI3,
  HID_EVENT_ID_EVT0,
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
  HID_EVENT_ID_EVT27
} hidEventId_t;

#define HID_DATA_SIZE 1

#if( 0 < HID_CONTROLS )

unsigned UserHIDGetData( void );
void UserHIDInit( void );
void UserHIDRegisterEvent( const hidEventId_t hidEventId, const int * hidEventData, const unsigned hidEventDataSize );

#endif /* ( 0 < HID_CONTROLS ) */
#endif /* __USER_HID_H__ */

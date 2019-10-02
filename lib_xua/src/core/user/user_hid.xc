// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdint.h>
#include <limits.h>
#include <xs1.h>
#include "user_hid.h"

#if( 0 < HID_CONTROLS )
#define HID_REPORT_INTERRUPT_ASSERTED   0x01
#define HID_REPORT_INTERRUPT_DEASSERTED 0x00

static unsigned char s_hidData;

void UserInitHIDData( void )
{
  s_hidData  = HID_REPORT_INTERRUPT_DEASSERTED;
}

void UserReadHIDData( unsigned char hidData[ HID_DATA_SIZE ])
{
  hidData[ 0 ] = s_hidData;

  for( unsigned i = 1; i < HID_DATA_SIZE; ++i ) {
    hidData[ i ] = 0U;
  }
}

void UserSetHIDData( const unsigned hidData )
{
  if( hidData == INT_ASSERT_LEVEL ) {
    s_hidData = HID_REPORT_INTERRUPT_ASSERTED;
  } else {
    s_hidData = HID_REPORT_INTERRUPT_DEASSERTED;
  }
}

#endif /* ( 0 < HID_CONTROLS ) */

// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdint.h>
#include <limits.h>
#include <xs1.h>
#include "user_hid.h"

#if( 0 < HID_CONTROLS )
#define HID_REPORT_INTERRUPT_ASSERTED   0x01
#define HID_REPORT_INTERRUPT_DEASSERTED 0x00

void UserReadHIDData( in port p_int, unsigned char hidData[ HID_DATA_SIZE ])
{
  unsigned curr_val;

  p_int :> curr_val;
  hidData[ 0 ] = ( curr_val == NDP10X_ASSERT_LEVEL ) ? HID_REPORT_INTERRUPT_ASSERTED : HID_REPORT_INTERRUPT_DEASSERTED;

  for( unsigned idx = 1; idx < HID_DATA_SIZE; ++idx ) {
    hidData[ idx ] = 0U;
  }
}

#endif /* ( 0 < HID_CONTROLS ) */

// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdint.h>
#include <limits.h>
#include <xs1.h>
#include "user_hid.h"

#if( 0 < HID_CONTROLS )
#if( 0 < HID_SIMULATE_NDP10X )

#define HID_REPORT_DATA 0x01

static unsigned char initialised = 0;

static unsigned int curr_time  = 0;
static unsigned int last_time  = 0;
static unsigned int tick_count = 0;

void UserReadHIDData( in port p_int, unsigned char hidData[ HID_DATA_SIZE ])
{
  timer tmr;

  if( !initialised ) {
    tmr :> last_time;
    initialised = 1;
  } else {
    tmr :> curr_time;
    tick_count += ( last_time < curr_time ) ? curr_time - last_time : curr_time + ( UINT_MAX - last_time );

    if(( HID_INTERRUPT_COUNT <= tick_count ) && ( tick_count <= ( HID_INTERRUPT_COUNT + HID_DEASSERT_COUNT ))) {
      for( unsigned idx = 0; idx < HID_DATA_SIZE; ++idx ) {
        hidData[ idx ] = HID_REPORT_DATA;
      }
    } else {
      for( unsigned idx = 0; idx < HID_DATA_SIZE; ++idx ) {
        hidData[ idx ] = 0x00;  
      }

      if (( HID_INTERRUPT_COUNT + HID_DEASSERT_COUNT ) <= tick_count ) {
        tick_count = 0;
      }
    }

    last_time = curr_time;
  }
}
#else  /* ( 0 < HID_SIMULATE_NDP10X ) */

#define HID_REPORT_INTERRUPT_ASSERTED   0x01
#define HID_REPORT_INTERRUPT_DEASSERTED 0x00

void UserReadHIDData( in port p_int, unsigned char hidData[ HID_DATA_SIZE ])
{
  unsigned curr_val;

  p_int :> curr_val;
  hidData[ 0 ] = ( curr_val == NDP100_ASSERT_LEVEL ) ? HID_REPORT_INTERRUPT_ASSERTED : HID_REPORT_INTERRUPT_DEASSERTED;

  for( unsigned idx = 1; idx < HID_DATA_SIZE; ++idx ) {
    hidData[ idx ] = 0U;
  }
}

#endif /* ( 0 < HID_SIMULATE_NDP10X ) */
#endif /* ( 0 < HID_CONTROLS ) */

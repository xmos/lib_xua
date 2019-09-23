// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdint.h>
#include <limits.h>
#include <xs1.h>
#include "user_hid.h"

void UserReadHIDData( unsigned char hidData[ HID_DATA_SIZE ])
{
  for( unsigned idx = 0; idx < HID_DATA_SIZE; ++idx ) {
    hidData[ idx ] = 1;
  }
}
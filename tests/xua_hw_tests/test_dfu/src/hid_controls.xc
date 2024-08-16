// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <platform.h>

#include "user_hid.h"
#include "xua_hid_report.h"

#if HID_CONTROLS > 0

size_t UserHIDGetData( const unsigned id, unsigned char hidData[ HID_MAX_DATA_BYTES ])
{
    hidData[0] = 0;
    return 1;
}

void UserHIDInit( void )
{
}

#endif  // HID_CONTROLS > 0
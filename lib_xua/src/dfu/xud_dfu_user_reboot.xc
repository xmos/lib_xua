// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "XUD_HAL.h"

void xud_dfu_user_pre_reboot(void)
{
    // Mark the device as off the bus, before we reboot.
    XUD_HAL_EnterMode_TristateDrivers();
}

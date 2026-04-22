// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xud_hal.h"

void xud_dfu_user_pre_reboot(void)
{
    XUD_HAL_EnterMode_TristateDrivers();
}

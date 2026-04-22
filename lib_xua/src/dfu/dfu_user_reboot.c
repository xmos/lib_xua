// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "dfu_reboot.h"

extern void xud_dfu_user_pre_reboot(void);

void dfu_user_pre_reboot(void)
{
    xud_dfu_user_pre_reboot();
}

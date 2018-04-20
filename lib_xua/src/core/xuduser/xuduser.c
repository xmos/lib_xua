// Copyright (c) 2013-2018, XMOS Ltd, All rights reserved
#if XUA_USB_EN
#include "xua.h"
#include "hostactive.h"
#include "audiostream.h"

void XUD_UserSuspend(void) __attribute__ ((weak));
void XUD_UserSuspend(void)
{
    UserAudioStreamStop();
    UserHostActive(0);
}

void XUD_UserResume(void) __attribute__ ((weak));
void XUD_UserResume(void)
{
    unsigned config;

    asm("ldw %0, dp[g_currentConfig]" : "=r" (config):);

    if(config == 1)
    {
        UserHostActive(1);
    }
}
#endif /* XUA_USB_EN*/

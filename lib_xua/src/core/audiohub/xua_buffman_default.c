// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xccompat.h"
#include "xua_audiohub.h"

/* Default implementation for UserBufferManagementInit() */
void __attribute__ ((weak)) UserBufferManagementInit()
{
    /* Do nothing */
}

/* Default implementation for UserBufferManagement() */
void __attribute__ ((weak)) UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[])
{
    /* Do nothing */
}

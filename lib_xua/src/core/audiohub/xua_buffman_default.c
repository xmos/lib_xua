// Copyright (c) 2016-2018, XMOS Ltd, All rights reserved

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

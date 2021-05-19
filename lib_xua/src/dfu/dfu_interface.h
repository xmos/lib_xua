// Copyright 2015-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __DFU_INTERFACE_H__
#define __DFU_INTERFACE_H__

#if __XC__

#include "xud_device.h"

interface i_dfu
{
    {unsigned, int, int, int, unsigned} HandleDfuRequest(USB_SetupPacket_t &sp, unsigned data_buffer[], unsigned data_buffer_length, unsigned dfuState);
    void finish();
};

#endif
#endif



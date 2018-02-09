// Copyright (c) 2015-2017, XMOS Ltd, All rights reserved

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



// Copyright 2015-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __DFU_INTERFACE_H__
#define __DFU_INTERFACE_H__

#if __XC__

#include <stdint.h>

interface i_dfu
{
    // TODO - fix the parameter lists
    {unsigned, int, int, int, unsigned} HandleDfuRequest(uint16_t request, uint16_t value, uint16_t index, uint16_t length, unsigned data_buffer[], unsigned data_buffer_length, unsigned dfuState);
    void finish();
};

#endif
#endif



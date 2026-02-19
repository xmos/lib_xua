// Copyright 2015-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __DFU_INTERFACE_H__
#define __DFU_INTERFACE_H__

#if __XC__

#include <stdint.h>

enum dfu_reset_type
{
    DFU_RESET_TYPE_NONE,
    DFU_RESET_TYPE_RESET_TO_DFU,
    DFU_RESET_TYPE_RESET_TO_APP
};

struct dfu_request_result
{
    int return_data_len;
    int return_code;
    enum dfu_reset_type reset_type;
    unsigned dfu_state;
};

struct dfu_request_params
{
    uint16_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
    unsigned dfu_state;
};

interface i_dfu
{
    struct dfu_request_result HandleDfuRequest(struct dfu_request_params request, unsigned data_buffer[], unsigned data_buffer_length);
    void finish();
};

#endif
#endif



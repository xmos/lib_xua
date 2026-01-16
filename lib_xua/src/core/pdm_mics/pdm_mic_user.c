// Copyright 2016-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xua.h"
#include "xua_conf_full.h"

#if (XUA_NUM_PDM_MICS > 0)
#include "xua_pdm_mic.h"

/* Deafult implementations of xua_user_pdm_init() and xua_user_pdm_process().  Both can be over-ridden */
void xua_user_pdm_init() __attribute__ ((weak));
void xua_user_pdm_init(unsigned channel_map[MIC_ARRAY_CONFIG_MIC_COUNT])
{
    return;
}


void xua_user_pdm_process() __attribute__ ((weak));
void xua_user_pdm_process(int32_t mic_audio[MIC_ARRAY_CONFIG_MIC_COUNT])
{
    return;
}

#endif

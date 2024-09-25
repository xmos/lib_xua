// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xua.h"
#include "xua_pdm_mic.h"
#include "xua_conf_full.h"

#if (XUA_NUM_PDM_MICS > 0) && !defined(MIC_PROCESSING_USE_INTERFACE)

/* Deafult implementations of user_pdm_init() and user_pdm_process().  Both can be over-ridden */
void user_pdm_init() __attribute__ ((weak));
void user_pdm_init()
{
    return;
}


void user_pdm_process() __attribute__ ((weak));
void user_pdm_process(int32_t audio[MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME][MIC_ARRAY_CONFIG_MIC_COUNT])
{
    return;
}

#endif

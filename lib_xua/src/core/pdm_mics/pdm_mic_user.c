// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xua.h"

#if (XUA_NUM_PDM_MICS > 0) && !defined(MIC_PROCESSING_USE_INTERFACE)

#include "mic_array_frame.h"

/* Deafult implementations of user_pdm_init() and user_pdm_process().  Both can be over-ridden */
void user_pdm_init() __attribute__ ((weak));
void user_pdm_init()
{
    return;
}


void user_pdm_process() __attribute__ ((weak));
void user_pdm_process(mic_array_frame_time_domain * audio)
{
    return;
}

#endif

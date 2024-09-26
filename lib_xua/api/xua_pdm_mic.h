// Copyright 2015-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XUA_PDM_MIC_H
#define XUA_PDM_MIC_H

#include <xccompat.h>
#include <stdint.h>

#include "mic_array_conf.h"
#include "mic_array.h"

#ifdef __cplusplus
extern "C" {
#endif
void ma_init(unsigned mic_samp_rate);
void ma_task(chanend c_mic_to_audio);
#ifdef __cplusplus
}
#endif

void mic_array_task(chanend c_mic_to_audio);
void user_pdm_init();
void user_pdm_process(int32_t mic_audio[MIC_ARRAY_CONFIG_MIC_COUNT]);

#endif


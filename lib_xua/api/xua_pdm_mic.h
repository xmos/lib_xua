// Copyright 2015-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XUA_PDM_MIC_H
#define XUA_PDM_MIC_H

#include "xua_conf.h"

#define MIC_ARRAY_CONFIG_MCLK_FREQ          MCLK_48
#define MIC_ARRAY_CONFIG_PDM_FREQ           3072000
#define MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME  1
#define MIC_ARRAY_CONFIG_PORT_MCLK          XS1_PORT_1D
#define MIC_ARRAY_CONFIG_PORT_PDM_CLK       XS1_PORT_1G
#define MIC_ARRAY_CONFIG_PORT_PDM_DATA      XS1_PORT_1F
#define MIC_ARRAY_CONFIG_USE_DDR            1
#define MIC_ARRAY_CONFIG_CLOCK_BLOCK_A      XS1_CLKBLK_1      
#define MIC_ARRAY_CONFIG_CLOCK_BLOCK_B      XS1_CLKBLK_2
#define appconfSHF_NOMINAL_HZ               MIN_FREQ
#define MIC_ARRAY_CONFIG_MIC_COUNT          XUA_NUM_PDM_MICS
#define MIC_ARRAY_CONFIG_MIC_IN_COUNT       XUA_NUM_PDM_MICS
#define MIC_ARRAY_CONFIG_USE_DC_ELIMINATION 1

/* Included from lib_mic_array */
#include <xccompat.h>


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


#endif


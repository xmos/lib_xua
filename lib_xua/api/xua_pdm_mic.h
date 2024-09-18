// Copyright 2015-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XUA_PDM_MIC_H
#define XUA_PDM_MIC_H



#define MIC_ARRAY_CONFIG_MCLK_FREQ          24576000
#define MIC_ARRAY_CONFIG_PDM_FREQ           3072000
#define MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME  1
#define MIC_ARRAY_CONFIG_PORT_MCLK          XS1_PORT_1D
#define MIC_ARRAY_CONFIG_PORT_PDM_CLK       XS1_PORT_1G
#define MIC_ARRAY_CONFIG_PORT_PDM_DATA      XS1_PORT_1F
#define MIC_ARRAY_CONFIG_USE_DDR            1
#define MIC_ARRAY_CONFIG_CLOCK_BLOCK_A      XS1_CLKBLK_1      
#define MIC_ARRAY_CONFIG_CLOCK_BLOCK_B      XS1_CLKBLK_2
#define appconfSHF_NOMINAL_HZ               16000
#define MIC_ARRAY_CONFIG_MIC_COUNT          2
#define MIC_ARRAY_CONFIG_MIC_IN_COUNT       1
#define MIC_ARRAY_CONFIG_USE_DC_ELIMINATION 1

/* Included from lib_mic_array */
#include <xccompat.h>


#include "mic_array.h"

#ifdef __cplusplus
extern "C" {
#endif
void ma_init();
void ma_task(chanend c_mic_to_audio);
#ifdef __cplusplus
}
#endif

void mic_array_task(chanend c_mic_to_audio);


#if 0

/* Configures PDM ports/clocks */
void xua_pdm_mic_config(in port p_pdm_mclk, in port p_pdm_clk, buffered in port:32 p_pdm_mics, clock clk_pdm);

#ifdef MIC_PROCESSING_USE_INTERFACE
/* Interface based user processing */
typedef interface mic_process_if
{
    void transfer_buffers(mic_array_frame_time_domain * unsafe audio);
    void init();
} mic_process_if;


[[combinable]]
void XUA_PdmBuffer(streaming chanend c_ds_output[2], chanend c_audio
#ifdef MIC_PROCESSING_USE_INTERFACE
   , client mic_process_if i_mic_process
#endif
);

[[combinable]]
void user_pdm_process(server mic_process_if i_mic_data);

/* PDM interface and decimation cores */
void xua_pdm_mic(streaming chanend c_ds_output[2], buffered in port:32 p_pdm_mics);

#else

/* Simple user hooks/call-backs */
void user_pdm_process(mic_array_frame_time_domain * unsafe audio);

void user_pdm_init();

/* PDM interface and decimation cores */
[[combinable]]
void XUA_PdmBuffer(streaming chanend c_ds_output[2], chanend c_audio);

/* PDM interface and decimation cores */
void xua_pdm_mic(streaming chanend c_ds_output[2], buffered in port:32 p_pdm_mics);

#endif

#endif

#endif


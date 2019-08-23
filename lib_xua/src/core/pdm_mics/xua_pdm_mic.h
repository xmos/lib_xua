// Copyright (c) 2015-2019, XMOS Ltd, All rights reserved

#include "mic_array.h"

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
void pdm_buffer(streaming chanend c_ds_output[2], chanend c_audio
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
void pdm_buffer(streaming chanend c_ds_output[2], chanend c_audio);

/* PDM interface and decimation cores */
void xua_pdm_mic(streaming chanend c_ds_output[2], buffered in port:32 p_pdm_mics);

#endif


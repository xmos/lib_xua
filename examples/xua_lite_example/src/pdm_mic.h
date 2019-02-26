#ifndef _PDM_MIC_H_
#define _PDM_MIC_H_

#include "mic_array.h"

void mic_array_decimator_set_samprate(const unsigned samplerate, int mic_decimator_fir_data_array[], mic_array_decimator_conf_common_t *dcc, mic_array_decimator_config_t dc[]);
void pdm_mic(streaming chanend c_ds_output, in buffered port:32 p_pdm_mics);
void mic_array_setup_ddr(clock pdmclk, clock pdmclk6, in port p_mclk,
                         out port p_pdm_clk, buffered in port:32 p_pdm_data,
                         int divide);

#endif

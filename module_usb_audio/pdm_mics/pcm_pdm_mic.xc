
#include "devicedefines.h"

#if (NUM_PDM_MICS > 0)

/* This file includes an example integration of lib_array_mic into USB Audio */

#include <platform.h>
#include <xs1.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xclib.h>
#include <stdint.h>

#include "mic_array.h"

#define MAX_DECIMATION_FACTOR 12

/* Hardware resources */
in port p_pdm_clk                = PORT_PDM_CLK;
in buffered port:32 p_pdm_mics   = PORT_PDM_DATA;
in port p_mclk                   = PORT_PDM_MCLK;
clock pdmclk                     = on tile[PDM_TILE]: XS1_CLKBLK_3;

/* User hooks */
unsafe void user_pdm_process(mic_array_frame_time_domain * unsafe audio, int output[]);
void user_pdm_init();

int data_0[4*THIRD_STAGE_COEFS_PER_STAGE * MAX_DECIMATION_FACTOR] = {0};
int data_1[4*THIRD_STAGE_COEFS_PER_STAGE * MAX_DECIMATION_FACTOR] = {0};

mic_array_frame_time_domain mic_audio[2];

void pdm_process(streaming chanend c_ds_output[2], chanend c_audio)
{
    unsigned buffer = 1;     // Buffer index
    int output[NUM_PDM_MICS];

    user_pdm_init();

    while(1)
    {
        unsigned samplerate;

        c_audio :> samplerate;

        unsigned decimationfactor = 96000/samplerate;

        unsafe
        {
            const int * unsafe fir_coefs[7] = {0, g_third_stage_div_2_fir, g_third_stage_div_4_fir, g_third_stage_div_6_fir, g_third_stage_div_8_fir, 0, g_third_stage_div_12_fir};

            mic_array_decimator_conf_common_t dcc = {MIC_ARRAY_MAX_FRAME_SIZE_LOG2, 1, 0, 0, decimationfactor, fir_coefs[decimationfactor/2], 0, 0, DECIMATOR_NO_FRAME_OVERLAP, 2};
            mic_array_decimator_config_t dc[2] = {{&dcc, data_0, {0, 0, 0, 0}, 4}, {&dcc, data_1, {0, 0, 0, 0}, 4}};
            mic_array_decimator_configure(c_ds_output, 2, dc);

        mic_array_init_time_domain_frame(c_ds_output, 2, buffer, mic_audio, dc);

        while(1)
        {
            mic_array_frame_time_domain * unsafe current = mic_array_get_next_time_domain_frame(c_ds_output, 2, buffer, mic_audio, dc);

            unsafe
            {
                int req;
                user_pdm_process(current, output);

                c_audio :> req;

                if(req)
                {
                    for(int i = 0; i < NUM_PDM_MICS; i++)
                    {
                        c_audio <: output[i];
                    }
                }
                else
                {
                    break;
                }
            }
            }
        }
    }
}

#if MAX_FREQ > 48000
#error MAX_FREQ > 48000 NOT CURRENTLY SUPPORTED
#endif

void pcm_pdm_mic(chanend c_pcm_out)
{
    streaming chan c_4x_pdm_mic_0, c_4x_pdm_mic_1;
    streaming chan c_ds_output[2];

    /* Note, this divide should be based on master clock freq */
    configure_clock_src_divide(pdmclk, p_mclk, 2);
    configure_port_clock_output(p_pdm_clk, pdmclk);
    configure_in_port(p_pdm_mics, pdmclk);
    start_clock(pdmclk);

    par
    {
        mic_array_pdm_rx(p_pdm_mics, c_4x_pdm_mic_0, c_4x_pdm_mic_1);
        mic_array_decimate_to_pcm_4ch(c_4x_pdm_mic_0, c_ds_output[0]);
        mic_array_decimate_to_pcm_4ch(c_4x_pdm_mic_1, c_ds_output[1]);
        pdm_process(c_ds_output, c_pcm_out);
    }
}

#endif

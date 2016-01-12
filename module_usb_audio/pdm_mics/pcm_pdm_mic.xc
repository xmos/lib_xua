
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

#include "fir_decimator.h"
#include "mic_array.h"

#define MAX_DECIMATION_FACTOR 12

/* Hardware resources */
in port p_pdm_clk                = PORT_PDM_CLK;
in buffered port:32 p_pdm_mics   = PORT_PDM_DATA;
#if AUDIO_IO_TILE != PDM_TILE
in port p_mclk                   = PORT_PDM_MCLK;
#else
// Use 'p_mclk_in' shared by I2S
#endif
clock pdmclk                     = on tile[PDM_TILE]: XS1_CLKBLK_4;

/* User hooks */
unsafe void user_pdm_process(frame_audio * unsafe audio, int output[]);
void user_pdm_init();

int data_0[4*THIRD_STAGE_COEFS_PER_STAGE * MAX_DECIMATION_FACTOR] = {0};
int data_1[4*THIRD_STAGE_COEFS_PER_STAGE * MAX_DECIMATION_FACTOR] = {0};

frame_audio mic_audio[2];

void pdm_process(streaming chanend c_ds_output[2], chanend c_audio)
{
    unsigned buffer = 1;     // Buffer index
    memset(mic_audio, sizeof(frame_audio), 0);
    int output[NUM_PDM_MICS];

    user_pdm_init();

    while(1)
    {
        unsigned samplerate;

        c_audio :> samplerate;

        unsigned decimationfactor = 96000/samplerate;

        unsafe
        {
            int * unsafe fir_coefs[7] = {0, g_third_48kHz_fir, g_third_24kHz_fir, g_third_16kHz_fir, g_third_12kHz_fir, 0, g_third_8kHz_fir};

            decimator_config_common dcc = {MAX_FRAME_SIZE_LOG2, 1, 0, 0, decimationfactor, fir_coefs[decimationfactor/2], 0, INT_MAX>>4};
            decimator_config dc[2] = {{&dcc, data_0, {0, 0, 0, 0}, 4}, {&dcc, data_1, {0, 0, 0, 0}, 4}};
            decimator_configure(c_ds_output, 2, dc);
        }

        decimator_init_audio_frame(c_ds_output, 2, buffer, mic_audio, DECIMATOR_NO_FRAME_OVERLAP);

        while(1)
        {
            frame_audio * unsafe current = decimator_get_next_audio_frame(c_ds_output, 2, buffer, mic_audio, 2);

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

#if MAX_FREQ > 48000
#error MAX_FREQ > 48000 NOT CURRENTLY SUPPORTED
#endif

void pcm_pdm_mic(chanend c_pcm_out)
{
    streaming chan c_4x_pdm_mic_0, c_4x_pdm_mic_1;
    streaming chan c_ds_output[2];

    /* TODO, always run mics at 3MHz */
    /* Assuming MCLK is 24.576 MHz */
    #if AUDIO_IO_TILE != PDM_TILE
    configure_clock_src_divide(pdmclk, p_mclk, 4);
    #else
    unsigned port_id, divide = 4;
    asm("ldw %0, dp[p_mclk_in]":"=r"(port_id));
    asm("setclk res[%0], %1"::"r"(pdmclk), "r"(port_id));
    asm("setd res[%0], %1"::"r"(pdmclk), "r"(divide));
    #endif
    configure_port_clock_output(p_pdm_clk, pdmclk);
    configure_in_port(p_pdm_mics, pdmclk);
    start_clock(pdmclk);

    par
    {
        pdm_rx(p_pdm_mics, c_4x_pdm_mic_0, c_4x_pdm_mic_1);
        decimate_to_pcm_4ch(c_4x_pdm_mic_0, c_ds_output[0]);
        decimate_to_pcm_4ch(c_4x_pdm_mic_1, c_ds_output[1]);
        pdm_process(c_ds_output, c_pcm_out);
    }
}

#endif

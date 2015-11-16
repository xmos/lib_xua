
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

/* Hardware resources */
in port p_pdm_clk                = PORT_PDM_CLK;
in buffered port:32 p_pdm_mics   = PORT_PDM_DATA;
in port p_mclk                   = PORT_PDM_MCLK;
clock pdmclk                     = on tile[PDM_TILE]: XS1_CLKBLK_3;

/* User hooks */
unsafe void user_pdm_process(frame_audio * unsafe audio, int output[]);
void user_pdm_init();

int data_0[4*COEFS_PER_PHASE*MAX_DECIMATION_FACTOR] = {0};
int data_1[4*COEFS_PER_PHASE*MAX_DECIMATION_FACTOR] = {0};

frame_audio mic_audio[2];         

void pdm_process(streaming chanend c_ds_output_0, streaming chanend c_ds_output_1, chanend c_audio)
{
    unsigned buffer = 1;     // Buffer index
    memset(mic_audio, sizeof(frame_audio), 0);
    int output[NUM_PDM_MICS];

    user_pdm_init();
    
    while(1)
    { 
        unsigned samplerate;

        c_audio :> samplerate;
        
        unsigned decimationfactor = 48000/samplerate;

        unsafe
        {
            decimator_config_common dcc = {FRAME_SIZE_LOG2, 1, 0, 0, decimationfactor, fir_coefs[decimationfactor], 0};
            decimator_config dc0 = {&dcc, data_0, {0, 0, 0, 0}};
            decimator_config dc1 = {&dcc, data_1, {0, 0, 0, 0}};
            decimator_configure(c_ds_output_0, c_ds_output_1, dc0, dc1);
        }

        decimator_init_audio_frame(c_ds_output_0, c_ds_output_1, buffer, mic_audio);

        while(1)
        {
            frame_audio * unsafe current = decimator_get_next_audio_frame(c_ds_output_0, c_ds_output_1, buffer, mic_audio);
        
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
#error NOT CURRENTLY SUPPORTED
#endif

void pcm_pdm_mic(chanend c_pcm_out)
{
    streaming chan c_multi_channel_pdm, c_sync, c_4x_pdm_mic_0, c_4x_pdm_mic_1;
    streaming chan c_ds_output_0, c_ds_output_1;
    streaming chan c_buffer_mic0, c_buffer_mic1;
   
    /* TODO, always run mics at 3MHz */ 
    configure_clock_src_divide(pdmclk, p_mclk, 2);
    configure_port_clock_output(p_pdm_clk, pdmclk);
    configure_in_port(p_pdm_mics, pdmclk);
    start_clock(pdmclk);

    par 
    {
        pdm_rx(p_pdm_mics, c_4x_pdm_mic_0, c_4x_pdm_mic_1);
        decimate_to_pcm_4ch(c_4x_pdm_mic_0, c_ds_output_0);
        decimate_to_pcm_4ch(c_4x_pdm_mic_1, c_ds_output_1);
        pdm_process(c_ds_output_0, c_ds_output_1, c_pcm_out);
    }
}

#endif

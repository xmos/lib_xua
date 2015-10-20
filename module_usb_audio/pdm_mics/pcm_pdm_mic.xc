/* This file includes an integration of lib_array_mic into USB Audio */

#include <platform.h>
#include <xs1.h>
#include <stdlib.h>
#include <print.h>
#include <stdio.h>
#include <string.h>
#include <xclib.h>
#include <stdint.h>
#include "devicedefines.h"

#include "fir_decimator.h"
#include "mic_array.h"
#include "mic_array_board_support.h"

#define FORM_BEAM 1

#if 1
in port p_pdm_clk                = PORT_PDM_CLK;
in buffered port:32 p_pdm_mics   = PORT_PDM_DATA;

in port p_mclk                  = PORT_PDM_MCLK;
clock mclk                      = on tile[PDM_TILE]: XS1_CLKBLK_1;
clock pdmclk                    = on tile[PDM_TILE]: XS1_CLKBLK_3;

void user_pdm_process(frame_audio *audio)
{
    static unsigned gain = 8*4096*8;

    for(unsigned i=0;i<7;i++)
    {
        unsigned output = audio->data[i][0];
        audio->data[i][0] = ((uint64_t)output*gain)>>8;
    }
}


void pdm_process(streaming chanend c_ds_output_0, streaming chanend c_ds_output_1, chanend c_audio)
{

    unsigned buffer = 1;     //buffer index
    frame_audio audio[2];    //double buffered
    memset(audio, sizeof(frame_audio), 0);

    decimator_init_audio_frame(c_ds_output_0, c_ds_output_1, buffer, audio);

    unsafe
    {
        while(1)
        {
            frame_audio *  current = decimator_get_next_audio_frame(c_ds_output_0, c_ds_output_1, buffer, audio);

            user_pdm_process(current);
               
            /* Send out the individual mics */ 
            for(unsigned i=0;i<7;i++)
            {
                unsigned output = current->data[i][0];
                c_audio <: output;
            }
            c_audio <: 0;
        }
    }
}

#define DF 1

#define OUTPUT_SAMPLE_RATE (48000/DF)
#define MASTER_CLOCK_FREQUENCY 24576000

//TODO make these not global
int data_0[4*COEFS_PER_PHASE*DF] = {0};
int data_1[4*COEFS_PER_PHASE*DF] = {0};

void pcm_pdm_mic(chanend c_pcm_out)
{
    streaming chan c_multi_channel_pdm, c_sync, c_4x_pdm_mic_0, c_4x_pdm_mic_1;
    streaming chan c_ds_output_0, c_ds_output_1;
    streaming chan c_buffer_mic0, c_buffer_mic1;
    
    configure_clock_src(mclk, p_mclk);
    configure_clock_src_divide(pdmclk, p_mclk, 2);
    configure_port_clock_output(p_pdm_clk, pdmclk);
    configure_in_port(p_pdm_mics, pdmclk);
    start_clock(mclk);
    start_clock(pdmclk);

    unsafe
    {
        decimator_config dc0 = {FRAME_SIZE_LOG2, 1, 0, 0, DF, FIR_LUT(DF), data_0, 0, {0,0, 0, 0}};
        decimator_config dc1 = {FRAME_SIZE_LOG2, 1, 0, 0, DF, FIR_LUT(DF), data_1, 0, {0,0, 0, 0}};

        par 
        {
            pdm_rx(p_pdm_mics, c_4x_pdm_mic_0, c_4x_pdm_mic_1);
            decimate_to_pcm_4ch(c_4x_pdm_mic_0, c_ds_output_0, dc0);
            decimate_to_pcm_4ch(c_4x_pdm_mic_1, c_ds_output_1, dc1);
            pdm_process(c_ds_output_0, c_ds_output_1, c_pcm_out);
        }

    }
}

#endif

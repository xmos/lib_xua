#include <xscope.h>
#include <platform.h>
#include <xs1.h>
#include <stdlib.h>
#include "beam.h"
#include <print.h>
#include <stdio.h>
#include <string.h>

#include "static_constants.h"
#if 1
on tile[0]: in port p_pdm_clk = XS1_PORT_1E;
on tile[0]: in buffered port:8 p_pdm_mics = XS1_PORT_8B;
in port p_mclk                 = on tile[0]: XS1_PORT_1F;
clock mclk                     = on tile[0]: XS1_CLKBLK_1;
clock pdmclk                   = on tile[0]: XS1_CLKBLK_3;

in port p_buttons = on tile[0]: XS1_PORT_4C;

typedef struct {
    unsigned ch_a;
    unsigned ch_b;
} double_packed_audio;

typedef struct {
    double_packed_audio data[4][1<<FRAME_SIZE_LOG2];
} synchronised_audio;


void example(streaming chanend c_ds_output_0, streaming chanend c_ds_output_1, streaming chanend c_pcm_out)
{


    unsigned buffer = 1;            //buffer index
    synchronised_audio audio[2];    //double buffered
    memset(audio, sizeof(synchronised_audio), 2);


    unsafe
    {
        c_ds_output_0 <: (synchronised_audio * unsafe)audio[0].data[0];
        c_ds_output_1 <: (synchronised_audio * unsafe)audio[0].data[2];

        while(1)
        {   

            schkct(c_ds_output_0, 8);
            schkct(c_ds_output_1, 8);

            c_ds_output_0 <: (synchronised_audio * unsafe)audio[buffer].data[0];
            c_ds_output_1 <: (synchronised_audio * unsafe)audio[buffer].data[2];

            buffer = 1 - buffer;

            //The data has already been bit reversed in the downsampler

            // audio[buffer] is good to go
            //xscope_int(0, audio[buffer].data[0][0].ch_a);
            //xscope_int(1, audio[buffer].data[0][0].ch_b);
            c_pcm_out :> unsigned req;
            c_pcm_out <: audio[buffer].data[1][0].ch_a;
            c_pcm_out <: audio[buffer].data[0][0].ch_b;
            //printintln(audio[buffer].data[0][0].ch_b);
        }
    }
}


void pcm_pdm_mic(streaming chanend c_pcm_out)
{
    streaming chan c_multi_channel_pdm, c_sync, c_4x_pdm_mic_0, c_4x_pdm_mic_1;
    streaming chan c_ds_output_0, c_ds_output_1;
    streaming chan c_buffer_mic0, c_buffer_mic1;
    unsigned long long shared_memory[2] = {0};

    configure_clock_src(mclk, p_mclk);
    configure_clock_src_divide(pdmclk, p_mclk, 2);
    configure_port_clock_output(p_pdm_clk, pdmclk);
    configure_in_port(p_pdm_mics, pdmclk);
    start_clock(mclk);
    start_clock(pdmclk);

    unsafe 
    {
        unsigned long long * unsafe p_shared_memory = shared_memory;
        par
        {

            //Input stage
            pdm_first_stage(p_pdm_mics, p_shared_memory,
                            PDM_BUFFER_LENGTH_LOG2, c_sync,
                            c_4x_pdm_mic_0, c_4x_pdm_mic_1);

            pdm_to_pcm_4x(c_4x_pdm_mic_0, c_ds_output_0);
                    pdm_to_pcm_4x(c_4x_pdm_mic_1, c_ds_output_1);

            example(c_ds_output_0, c_ds_output_1, c_pcm_out);

        }
    }
}

#endif

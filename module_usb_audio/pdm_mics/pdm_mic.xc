
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
#include <assert.h>

#include "mic_array.h"
#include "xua_pdm_mic.h"

#define MAX_DECIMATION_FACTOR 12

/* Hardware resources */
in port p_pdm_clk                = PORT_PDM_CLK;
in buffered port:32 p_pdm_mics   = PORT_PDM_DATA;
in port p_mclk                   = PORT_PDM_MCLK;
clock pdmclk                     = on tile[PDM_TILE]: XS1_CLKBLK_3;

int data_0[4*THIRD_STAGE_COEFS_PER_STAGE * MAX_DECIMATION_FACTOR] = {0};
int data_1[4*THIRD_STAGE_COEFS_PER_STAGE * MAX_DECIMATION_FACTOR] = {0};

mic_array_frame_time_domain mic_audio[2];

#ifdef MIC_PROCESSING_USE_INTERFACE
[[combinable]]
#pragma unsafe arrays
void pdm_buffer(streaming chanend c_ds_output[2], chanend c_audio, client mic_process_if i_mic_process)
#else
#pragma unsafe arrays
void pdm_buffer(streaming chanend c_ds_output[2], chanend c_audio)
#endif
{
    unsigned buffer;
    int output[NUM_PDM_MICS];
    unsigned samplerate;

#ifdef MIC_PROCESSING_USE_INTERFACE
    i_mic_process.init();
#else
    user_pdm_init();
#endif

#if NUM_PDM_MICS > 4
    unsigned decimatorCount = 2;
#else
    unsigned decimatorCount = 1;
#endif

    mic_array_decimator_conf_common_t dcc;
    const int * unsafe fir_coefs[7];
    mic_array_frame_time_domain * unsafe current;
    mic_array_decimator_config_t dc[2];

    /* Get initial sample-rate and compute decimation factor */
    c_audio :> samplerate;
    unsigned decimationfactor = 96000/samplerate;

    int fir_gain_compen[7];

    unsafe
    {
        fir_gain_compen[0] = 0;
        fir_gain_compen[1] = FIR_COMPENSATOR_DIV_2;
        fir_gain_compen[2] = FIR_COMPENSATOR_DIV_4;
        fir_gain_compen[3] = FIR_COMPENSATOR_DIV_6;
        fir_gain_compen[4] = FIR_COMPENSATOR_DIV_6;
        fir_gain_compen[5] = 0;
        fir_gain_compen[6] = FIR_COMPENSATOR_DIV_12;

        fir_coefs[0] = 0;
        fir_coefs[1] = g_third_stage_div_2_fir;
        fir_coefs[2] = g_third_stage_div_4_fir;
        fir_coefs[3] = g_third_stage_div_6_fir;
        fir_coefs[4] = g_third_stage_div_8_fir;
        fir_coefs[5] = 0;
        fir_coefs[6] = g_third_stage_div_12_fir;

        //dcc = {MIC_ARRAY_MAX_FRAME_SIZE_LOG2, 1, 0, 0, decimationfactor, fir_coefs[decimationfactor/2], 0, 0, DECIMATOR_NO_FRAME_OVERLAP, 2};
        dcc.frame_size_log2 = MIC_ARRAY_MAX_FRAME_SIZE_LOG2;
        dcc.apply_dc_offset_removal = 1;
        dcc.index_bit_reversal = 0;
        dcc.windowing_function = null;
        dcc.output_decimation_factor = decimationfactor;
        dcc.coefs = fir_coefs[decimationfactor/2];
        dcc.apply_mic_gain_compensation = 0;
        dcc.fir_gain_compensation = fir_gain_compen[decimationfactor/2];
        dcc.buffering_type = DECIMATOR_NO_FRAME_OVERLAP;
        dcc.number_of_frame_buffers = 2;

        //dc[2] = {{&dcc, data_0, {0, 0, 0, 0}, 4}, {&dcc, data_1, {0, 0, 0, 0}, 4}};
        dc[0].dcc = &dcc;
        dc[0].data = data_0;
        dc[0].mic_gain_compensation[0]=0;
        dc[0].mic_gain_compensation[1]=0;
        dc[0].mic_gain_compensation[2]=0;
        dc[0].mic_gain_compensation[3]=0;
        dc[0].channel_count = 4;
        dc[1].dcc = &dcc;
        dc[1].data = data_1;
        dc[1].mic_gain_compensation[0]=0;
        dc[1].mic_gain_compensation[1]=0;
        dc[1].mic_gain_compensation[2]=0;
        dc[1].mic_gain_compensation[3]=0;
        dc[1].channel_count = 4;

        mic_array_decimator_configure(c_ds_output, decimatorCount, dc);

        mic_array_init_time_domain_frame(c_ds_output, decimatorCount, buffer, mic_audio, dc);

        /* Grab a first frame of mic data */
        /* Note, loop is unrolled once - allows for while(1) select {} and thus combinable */
        current = mic_array_get_next_time_domain_frame(c_ds_output, decimatorCount, buffer, mic_audio, dc);
    }

    /* Run user code */
    /* TODO ideally processing done inplace - it then doesn't matter if it is run or not */
#ifdef MIC_PROCESSING_USE_INTERFACE
    i_mic_process.transfer_buffers(current, output);
#else
    user_pdm_process(current, output);
#endif
    int req;
    while(1)
    {
        select
        {
            case c_audio :> req:

                /* Audio IO core requests samples */
                if(req)
                unsafe{
                    slave
                    {
#pragma loop unroll
                        for(int i = 0; i < NUM_PDM_MICS; i++)
                        {
                            c_audio <: output[i];
                        }
                    }

                    /* Get a new frame of mic data */
                    mic_array_frame_time_domain * unsafe current = mic_array_get_next_time_domain_frame(c_ds_output, decimatorCount, buffer, mic_audio, dc);

                    /* Run user code */
#ifdef MIC_PROCESSING_USE_INTERFACE
                    i_mic_process.transfer_buffers(current, output);
#else
                    user_pdm_process(current, output);
#endif
                }
                else
                unsafe{
                    /* Sample rate change */
                    c_audio :> samplerate;

                    /* Re-config the mic decimators for the new sample-rate */
                    decimationfactor = 96000/samplerate;
                    dcc.output_decimation_factor = decimationfactor;
                    dcc.coefs=fir_coefs[decimationfactor/2];
                    dcc.fir_gain_compensation = fir_gain_compen[decimationfactor/2];
                    mic_array_decimator_configure(c_ds_output, decimatorCount, dc);
                    mic_array_init_time_domain_frame(c_ds_output, decimatorCount, buffer, mic_audio, dc);

                    /* Get a new mic data frame */
                    mic_array_frame_time_domain * unsafe current = mic_array_get_next_time_domain_frame(c_ds_output, decimatorCount, buffer, mic_audio, dc);

                    /* Run user code */
#ifdef MIC_PROCESSING_USE_INTERFACE
                    i_mic_process.transfer_buffers(current, output);
#else
                    user_pdm_process(current, output);
#endif
                }
                break;
        } /* select */
    } /* while(1) */
}

#if MAX_FREQ > 48000
#error MAX_FREQ > 48000 NOT CURRENTLY SUPPORTED
#endif

#include "print.h"

void pdm_mic(streaming chanend c_ds_output[2])
{
    streaming chan c_4x_pdm_mic_0;
#if (NUM_PDM_MICS > 4)
    streaming chan c_4x_pdm_mic_1;
#else
    #define c_4x_pdm_mic_1 null
#endif

    /* Mics expect a clock in the 3Mhz range, calculate the divide based on mclk */
    /* e.g. For a 48kHz range mclk we expect a 3072000Hz mic clock */
    /* e.g. For a 44.1kHz range mclk we expect a 2822400Hz mic clock */

    /* Note, codebase currently does not handle a different divide for each clock */
    assert((MCLK_48 / 3072000) == (MCLK_441 / 2822400));
       
    unsigned micDiv = MCLK_48/3072000;
    
    configure_clock_src_divide(pdmclk, p_mclk, micDiv/2);
    configure_port_clock_output(p_pdm_clk, pdmclk);
    configure_in_port(p_pdm_mics, pdmclk);
    start_clock(pdmclk);

    par
    {
        mic_array_pdm_rx(p_pdm_mics, c_4x_pdm_mic_0, c_4x_pdm_mic_1);
        mic_array_decimate_to_pcm_4ch(c_4x_pdm_mic_0, c_ds_output[0]);
#if (NUM_PDM_MICS > 4)
        mic_array_decimate_to_pcm_4ch(c_4x_pdm_mic_1, c_ds_output[1]);
#endif
    }
}
#endif

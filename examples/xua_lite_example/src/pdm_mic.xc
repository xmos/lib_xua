#include "xua.h"
#if (XUA_NUM_PDM_MICS > 0)

#include <platform.h>
#include <xs1.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xclib.h>
#include <stdint.h>
#include <assert.h>

#include "mic_array.h"


void mic_array_decimator_set_samprate(const unsigned samplerate, int fir_gain_compen[], int * unsafe fir_coefs[], int mic_decimator_fir_data_array[], mic_array_decimator_conf_common_t *dcc, mic_array_decimator_config_t dc[])
{
    unsigned decimationfactor = 96000/samplerate;
    unsafe
    {
        fir_gain_compen[0] = 0;
        fir_gain_compen[1] = FIR_COMPENSATOR_DIV_2; //48kHz
        fir_gain_compen[2] = FIR_COMPENSATOR_DIV_4;
        fir_gain_compen[3] = FIR_COMPENSATOR_DIV_6; //16kHz
        fir_gain_compen[4] = FIR_COMPENSATOR_DIV_8;
        fir_gain_compen[5] = 0;
        fir_gain_compen[6] = FIR_COMPENSATOR_DIV_12;

        fir_coefs[0] = 0;
        fir_coefs[1] = (int * unsafe)g_third_stage_div_2_fir;
        fir_coefs[2] = (int * unsafe)g_third_stage_div_4_fir;
        fir_coefs[3] = (int * unsafe)g_third_stage_div_6_fir;
        fir_coefs[4] = (int * unsafe)g_third_stage_div_8_fir;
        fir_coefs[5] = 0;
        fir_coefs[6] = (int * unsafe)g_third_stage_div_12_fir;

        //dcc = {MIC_ARRAY_MAX_FRAME_SIZE_LOG2, 1, 0, 0, decimationfactor, fir_coefs[decimationfactor/2], 0, 0, DECIMATOR_NO_FRAME_OVERLAP, 2};
        dcc->frame_size_log2 = MIC_ARRAY_MAX_FRAME_SIZE_LOG2;
        dcc->apply_dc_offset_removal = 1;
        dcc->index_bit_reversal = 0;
        dcc->windowing_function = null;
        dcc->output_decimation_factor = decimationfactor;
        dcc->coefs = fir_coefs[decimationfactor/2];
        dcc->apply_mic_gain_compensation = 0;
        dcc->fir_gain_compensation = fir_gain_compen[decimationfactor/2];
        dcc->buffering_type = DECIMATOR_NO_FRAME_OVERLAP;
        dcc->number_of_frame_buffers = 2;

        //dc[0] = {&dcc, mic_decimator_fir_data[0], {0, 0, 0, 0}, 4};
        dc[0].dcc = dcc;
        dc[0].data = mic_decimator_fir_data_array;
        dc[0].mic_gain_compensation[0]=0;
        dc[0].mic_gain_compensation[1]=0;
        dc[0].mic_gain_compensation[2]=0;
        dc[0].mic_gain_compensation[3]=0;
        dc[0].channel_count = 4;
    }
}

#if MAX_FREQ > 48000
#error MAX_FREQ > 48000 NOT CURRENTLY SUPPORTED
#endif

void pdm_mic(streaming chanend c_ds_output, in buffered port:32 p_pdm_mics)
{
    streaming chan c_4x_pdm_mic_0;
    assert((MCLK_48 / 3072000) == (MCLK_441 / 2822400));
    par
    {
        mic_array_pdm_rx(p_pdm_mics, c_4x_pdm_mic_0, null);
        mic_array_decimate_to_pcm_4ch(c_4x_pdm_mic_0, c_ds_output, MIC_ARRAY_NO_INTERNAL_CHANS);

    }
}
#endif

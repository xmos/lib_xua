// Copyright 2017-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* A very simple *example* of a USB audio application (and as such is un-verified for production)
 *
 * It uses the main blocks from the lib_xua with the addition of PDM mic support using lib_mic_array
 *
 * - No DFU
 *
 */

#include <xs1.h>
#include <platform.h>

#include "xua.h"
#include "xud_device.h"

/* From lib_mic_array */
#include "mic_array.h"

/* Lib_mic_array declarations. Note, the defines derived from the xn file */
in port p_pdm_clk                = PORT_PDM_CLK;               /* Port for PDM mic clock */
in port p_pdm_mclk               = PORT_PDM_MCLK;              /* Master clock for PDM mics */

in buffered port:32 p_pdm_mics   = PORT_PDM_DATA;              /* Port for PDM mic data */

clock clk_pdm                    = on tile[0]: XS1_CLKBLK_1;   /* Clock-block for PDM mics */


/* Lib_xua port declarations. Note, the defines come from the xn file */
in port p_mclk_in                = PORT_MCLK_IN;               /* Master clock for the audio IO tile */

/* Resources for USB feedback */
in port p_for_mclk_count         = PORT_MCLK_COUNT;            /* Extra port for counting master clock ticks */

/* Clock-block declarations */
clock clk_audio_mclk             = on tile[1]: XS1_CLKBLK_1;   /* Master clock */

/* Endpoint type tables - informs XUD what the transfer types for each Endpoint in use and also
 * if the endpoint wishes to be informed of USB bus resets */
XUD_EpType epTypeTableOut[]   = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};
XUD_EpType epTypeTableIn[]    = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};

int main()
{
    /* Channels for lib_xud */
    chan c_ep_out[2];
    chan c_ep_in[2];

    /* Channel for communicating SOF notifications from XUD to the Buffering cores */
    chan c_sof;

    /* Channel for audio data between buffering cores and AudioHub/IO core */
    chan c_aud;

    /* Channel for communicating control messages from EP0 to the rest of the device (via the buffering cores) */
    chan c_aud_ctl;

    /* Array of channels for communication between PDM mic decimator task(s) and the XUA mic buffer task */
    streaming chan c_ds_output[2];

    /* Channel for communcation between XUA_AudioHub() and the XUA mic buffer task */
    chan c_mic_pcm;

    par
    {
        /* Low level USB device layer core */
        on tile[1]: XUD_Main(c_ep_out, 2, c_ep_in, 2, c_sof, epTypeTableOut, epTypeTableIn, XUD_SPEED_HS, XUD_PWR_BUS);

        /* Endpoint 0 core from lib_xua */
        /* Note, since we are not using many features we pass in null for quite a few params.. */
        on tile[1]: XUA_Endpoint0(c_ep_out[0], c_ep_in[0], c_aud_ctl, null, null, null, null);

        on tile[1]:
        {
            /* Connect master-clock clock-block to clock-block pin */
            set_clock_src(clk_audio_mclk, p_mclk_in);             /* Clock clock-block from mclk pin */
            set_port_clock(p_for_mclk_count, clk_audio_mclk);     /* Clock the "count" port from the clock block */
                                                                  /* Note, AudioHub() will start the clock */

           par
           {
                /* Buffering task - handles audio data to/from EP's and gives/gets data to/from the audio I/O core */
                /* Note, this spawns two cores */
                XUA_Buffer(c_ep_out[1], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);

                /* AudioHub/IO core does most of the audio IO i.e. I2S (also serves as a hub for all audio) */
                /* Note, since we are not using I2S we pass in null for LR and Bit clock ports and the I2S dataline ports */
                XUA_AudioHub(c_aud, clk_audio_mclk, null, p_mclk_in, null, null, null, null, c_mic_pcm);
            }
        }

        /* Microphone related tasks */
        on tile[0]:
        {
            streaming chan c_4x_pdm_mic_0;
            streaming chan c_4x_pdm_mic_1;

            configure_clock_src_divide(clk_pdm, p_pdm_mclk, 4);  /* Master clock to PDM clock divide */
            configure_port_clock_output(p_pdm_clk, clk_pdm);
            configure_in_port(p_pdm_mics, clk_pdm);
            start_clock(clk_pdm);

            par
            {
                /* PDM receive I/O task */
                mic_array_pdm_rx(p_pdm_mics, c_4x_pdm_mic_0, c_4x_pdm_mic_1);

                /* Run two decimator tasks for 8 mics */
                mic_array_decimate_to_pcm_4ch(c_4x_pdm_mic_0, c_ds_output[0], MIC_ARRAY_NO_INTERNAL_CHANS);
                mic_array_decimate_to_pcm_4ch(c_4x_pdm_mic_1, c_ds_output[1], MIC_ARRAY_NO_INTERNAL_CHANS);

                XUA_PdmBuffer(c_ds_output, c_mic_pcm);
            }
        }
    }

    return 0;
}



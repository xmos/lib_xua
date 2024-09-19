// Copyright 2017-2024 XMOS LIMITED.
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

#include "xk_evk_xu316/board.h"


/* Lib_mic_array declarations. Note, the defines derived from the xn file */
// in port p_pdm_clk                = PORT_PDM_CLK;               /* Port for PDM mic clock */
in port p_pdm_mclk               = PORT_MCLK_IN;               /* Master clock for PDM mics */

// in buffered port:32 p_pdm_mics   = PORT_PDM_DATA;              /* Port for PDM mic data */


/* Lib_xua port declarations. Note, the defines come from the xn file */
in port p_mclk_in                = on tile[0]: XS1_PORT_1D;               /* Master clock for the audio IO tile */

/* Resources for USB feedback */
in port p_for_mclk_count         = on tile[0]: XS1_PORT_16B;               /* Extra port for counting master clock ticks */

/* Clock-block declarations */
clock clk_audio_mclk_usb         = on tile[0]: XS1_CLKBLK_1;   /* Master clock */
clock clk_audio_mclk             = on tile[1]: XS1_CLKBLK_3;   /* I2S */


// I2S resources - TODO remove as not needed!
clock clk_audio_bclk                = on tile[1]: XS1_CLKBLK_4;   /* Bit clock */
buffered out port:32 p_i2s_adc[]    = {PORT_I2S_ADC_DATA};   /* I2S Data-line(s) */
buffered out port:32 p_i2s_dac[]    = {PORT_I2S_DAC_DATA};   /* I2S Data-line(s) */
buffered out port:32 p_lrclk        = PORT_I2S_LRCLK;    /* I2S Bit-clock */
buffered out port:32 p_bclk         = PORT_I2S_BCLK;     /* I2S L/R-clock */


void pdm_dummy(chanend c_mic_pcm, chanend c_mic_to_audio){
    int sample_rate = 0;
    c_mic_pcm :> sample_rate;
    printintln(sample_rate);

    int wave = 0;

    // Synch
    int tt;
    c_mic_to_audio :> tt;
    printintln(tt);

    // delay_milliseconds(100);
    // int32_t mic_samps[MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME][MIC_ARRAY_CONFIG_MIC_COUNT];                
    int32_t mic_samps[2][2];                


    while(1){
        // while(1){
            unsafe{
                chanend_t c_m2a = (chanend_t)c_mic_to_audio;
                ma_frame_rx(mic_samps[0], c_m2a, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME, MIC_ARRAY_CONFIG_MIC_COUNT);
                
                static int cc = 0; if(cc++ > 16000){printchar('+');cc = 0;}
            }
        // }

        int transfer = 0;
        c_mic_pcm :> transfer;
        if(transfer){
            slave
            {
                // Horrible triangluar wave generator @80Hz
                int sample = (wave * 1000000);
                for(int i = 0; i < XUA_NUM_PDM_MICS; i++)
                {
                    c_mic_pcm <: mic_samps[0][i];
                    // c_mic_pcm <: sample;
                }
                if(++wave > 100){
                    // printstr(".");
                     wave = -100;
                }
                // printintln(sample);
            }
            static int ic = 0; if(ic++ > 16000){printchar('.');ic = 0;}

        } else {
            c_mic_pcm :> sample_rate;
            printintln(sample_rate);
        }
    }
}


void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[]){
    // printintln(sampsFromAudioToUsb[0]);
    sampsFromUsbToAudio[0] = sampsFromAudioToUsb[0]; // Copy to DAC so we can monitor
    sampsFromUsbToAudio[1] = sampsFromAudioToUsb[1]; // Copy to DAC so we can monitor
}

// Board configuration from lib_board_support
static const xk_evk_xu316_config_t hw_config = {
        MCLK_48// default_mclk
};

/* Endpoint type tables - informs XUD what the transfer types for each Endpoint in use and also
 * if the endpoint wishes to be informed of USB bus resets */
XUD_EpType epTypeTableOut[]   = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};
XUD_EpType epTypeTableIn[]    = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO, XUD_EPTYPE_ISO};

int main()
{
    /* Channels for lib_xud */
    chan c_ep_out[2];
    chan c_ep_in[3];

    /* Channel for communicating SOF notifications from XUD to the Buffering cores */
    chan c_sof;

    /* Channel for audio data between buffering cores and AudioHub/IO core */
    chan c_aud;

    /* Channel for communicating control messages from EP0 to the rest of the device (via the buffering cores) */
    chan c_aud_ctl;


    /* Channel for communcation between XUA_AudioHub() and the XUA mic buffer task */
    chan c_mic_pcm;

    chan c_i2c;

    par
    {
        /* Low level USB device layer core */
        on tile[0]: XUD_Main(c_ep_out, 2, c_ep_in, 3, c_sof, epTypeTableOut, epTypeTableIn, XUD_SPEED_HS, XUD_PWR_BUS);

        /* Endpoint 0 core from lib_xua */
        /* Note, since we are not using many features we pass in null for quite a few params.. */
        on tile[0]: XUA_Endpoint0(c_ep_out[0], c_ep_in[0], c_aud_ctl, null, null, null, null);

        on tile[0]:
        {
            /* Connect master-clock clock-block to clock-block pin */
            set_clock_src(clk_audio_mclk_usb, p_mclk_in);             /* Clock clock-block from mclk pin */
            set_port_clock(p_for_mclk_count, clk_audio_mclk_usb);     /* Clock the "count" port from the clock block */
            start_clock(clk_audio_mclk_usb);

           par
           {
                /* Buffering task - handles audio data to/from EP's and gives/gets data to/from the audio I/O core */
                /* Note, this spawns two cores */
                XUA_Buffer(c_ep_out[1], c_ep_in[2], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);
                xk_evk_xu316_AudioHwRemote(c_i2c); // Startup remote I2C master server task

            }
        }

        on tile[1]:
        {
            xk_evk_xu316_AudioHwChanInit(c_i2c);
            xk_evk_xu316_AudioHwInit(hw_config);
            xk_evk_xu316_AudioHwConfig(48000, hw_config.default_mclk, 0, 24, 24);

            chan c_mic_to_audio;

            par
            {
                /* AudioHub/IO core does most of the audio IO i.e. I2S (also serves as a hub for all audio) */
                /* Note, since we are not using I2S we pass in null for LR and Bit clock ports and the I2S dataline ports */
                // XUA_AudioHub(c_aud, clk_audio_mclk, clk_audio_bclk, p_pdm_mclk, p_lrclk, p_bclk, p_i2s_dac, null);
                XUA_AudioHub(c_aud, clk_audio_mclk, clk_audio_bclk, p_pdm_mclk, p_lrclk, p_bclk, p_i2s_dac, null, c_mic_pcm);

                /* Microphone related tasks */
                pdm_dummy(c_mic_pcm, c_mic_to_audio);
                mic_array_task(c_mic_to_audio);
            }
        }
    }

    return 0;
}



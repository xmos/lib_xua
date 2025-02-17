// Copyright 2017-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* A very simple *example* of a USB audio application (and as such is un-verified for production)
 *
 * It uses the main blocks from the lib_xua
 *
 * - 2 channels USB Audio in
 * - 2 channels I2S out
 * - No DFU
 * - PDM mics enable
 *
 */

#include <xs1.h>
#include <platform.h>
#include <string.h>

#include "xua.h"
#include "xud_device.h"

#include "xk_evk_xu316/board.h"

#if 0
/* Port declarations. Note, the defines come from the xn file */
buffered out port:32 p_i2s_dac[]    = {PORT_I2S_DAC_DATA};   /* I2S Data-line(s) */
buffered out port:32 p_lrclk        = PORT_I2S_LRCLK;    /* I2S Bit-clock */
buffered out port:32 p_bclk         = PORT_I2S_BCLK;     /* I2S L/R-clock */

/* Master clock for the audio IO tile */
in port p_mclk_in                   = PORT_MCLK_IN;

/* Resources for USB feedback */
in port p_for_mclk_count            = on tile[0]: XS1_PORT_16B;   /* Extra port for counting master clock ticks */
in port p_mclk_in_usb               = on tile[0]: XS1_PORT_1D;    /* Extra master clock input for the USB tile - looped back on hardware */

/* Clock-block declarations */
clock clk_audio_bclk                = on tile[1]: XS1_CLKBLK_4;   /* Bit clock */
clock clk_audio_mclk                = on tile[1]: XS1_CLKBLK_5;   /* Master clock */
clock clk_audio_mclk_usb            = on tile[0]: XS1_CLKBLK_1;   /* Master clock for USB tile */

/* Endpoint type tables - informs XUD what the transfer types for each Endpoint in use and also
 * if the endpoint wishes to be informed of USB bus resets */
XUD_EpType epTypeTableIn[]    = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};
XUD_EpType epTypeTableOut[]   = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE};

/* Port declarations for I2C to config DAC */
on tile[0]: port p_scl = XS1_PORT_1L;
on tile[0]: port p_sda = XS1_PORT_1M;

#endif

/* Make a copy of inbound mic samples and send to DAC */
#pragma unsafe arrays
void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[])
{

    for(int i = 0; i < I2S_CHANS_DAC; i++)
    {
        sampsFromUsbToAudio[i] = sampsFromAudioToUsb[i];
    }
}

/* Apply some gain so we can hear the mics easily (non-saturating - will overflow) */
#pragma unsafe arrays
void user_pdm_process(int32_t mic_audio[MIC_ARRAY_CONFIG_MIC_COUNT])
{
    for(int i = 0; i < XUA_NUM_PDM_MICS; i++)
    {
        mic_audio[i] = mic_audio[i] << 6; /* x64 */
    }
}

#if 0

int main()
{
    /* Channels for lib_xud */
    chan c_ep_out[1];
    chan c_ep_in[2];

    /* Channel for communicating SOF notifications from XUD to the Buffering cores */
    chan c_sof;

    /* Channel for audio data between buffering cores and AudioHub/IO core */
    chan c_aud;

    /* Channel for communicating control messages from EP0 to the rest of the device (via the buffering cores) */
    chan c_aud_ctl;

    /* Channel for communicating I2C control messages from audio to the I2C master server */
    chan c_i2c;

    /* Channel for communicating between mic_array and audio */
    chan c_mic_pcm;


    par
    {
        /* Low level USB device layer core */
        on tile[0]: XUD_Main(c_ep_out, 1, c_ep_in, 2,
                      c_sof, epTypeTableOut, epTypeTableIn,
                      XUD_SPEED_FS, XUD_PWR_BUS);

        /* Endpoint 0 core from lib_xua */
        /* Note, since we are not using many features we pass in null for quite a few params.. */
        on tile[0]: XUA_Endpoint0(c_ep_out[0], c_ep_in[0], c_aud_ctl, null, null, null, null);

        /* Buffering cores - handles audio data to/from EP's and gives/gets data to/from the audio I/O core */
        /* Note, this spawns two cores */
        on tile[0]: {

                        /* Connect master-clock clock-block to clock-block pin */
                        set_clock_src(clk_audio_mclk_usb, p_mclk_in_usb);           /* Clock clock-block from mclk pin */
                        set_port_clock(p_for_mclk_count, clk_audio_mclk_usb);       /* Clock the "count" port from the clock block */
                        start_clock(clk_audio_mclk_usb);                            /* Set the clock off running */

                        par{
                            XUA_Buffer(c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);
                            xk_evk_xu316_AudioHwRemote(c_i2c); // Startup remote I2C master server task
                        }
                    }

        on tile[1]:
        {
            xk_evk_xu316_AudioHwChanInit(c_i2c);

            par
            {
                /* AudioHub/IO core does most of the audio IO i.e. I2S (also serves as a hub for all audio including mics) */
                XUA_AudioHub(c_aud, clk_audio_mclk, clk_audio_bclk, p_mclk_in, p_lrclk, p_bclk, p_i2s_dac, null, c_mic_pcm);

                /* Microphone task */
                mic_array_task(c_mic_pcm);
            }
        }
    } /* par */

    return 0;
}


#endif
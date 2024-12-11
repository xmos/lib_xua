// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* A very simple *example* of a USB audio application (and as such is
 * un-verified for production)
 *
 * It uses the following features of lib_xua:
 *  - 2 channels USB Audio In
 *  - PDM microphones enabled
 *  - No DFU
 *
 * This example is designed to run on the xcore.ai Vision Development Kit.
 *
 * */

/*** Includes ***/

/* C standard library includes */
#include <string.h>

/* Toolchain includes */
#include <platform.h>
#include <xs1.h>

/* Library includes */
#include <xua.h>
#include <xud_device.h>

extern "C"
{
#include <sw_pll.h>
}

/*** Global declarations ***/

/* Endpoint type tables - informs XUD what the transfer types for each Endpoint
 * in use and also if the endpoint wishes to be informed of USB bus resets */
XUD_EpType epTypeTableIn[]  = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE,
                               XUD_EPTYPE_ISO};
XUD_EpType epTypeTableOut[] = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE};

#if USE_APP_PLL_TILE_0
/* Extra master clock input for the USB tile - looped back on hardware */
in port p_mclk_in_usb       = on tile[XUD_TILE]: USB_MCLK_PORT; 

/* Extra port for counting master clock ticks */
in port p_for_mclk_count    = on tile[XUD_TILE]: USB_MCLK_COUNT_PORT;
#define COUNT_PORT p_for_mclk_count

/* Master clock for USB tile */
clock clk_audio_mclk_usb    = on tile[XUD_TILE]: USB_MCLK_CLOCK_BLOCK;
#else
/* No port needed for counting ticks - make it null instead */
#define COUNT_PORT null
#endif

/*** User functions ***/

/* Apply some gain so we can hear the mics (non-saturating - will overflow) */
#pragma unsafe arrays
void user_pdm_process(int32_t mic_audio[MIC_ARRAY_CONFIG_MIC_COUNT])
{
    for (int i = 0; i < XUA_NUM_PDM_MICS; i++)
    {
        mic_audio[i] = mic_audio[i] << 6; /* x64 */
    }
}

/*** Main ***/

int main()
{
    /* Channels for lib_xud */
    chan c_ep_out[1];
    chan c_ep_in[2];

    /* Channel for sending SOF notifications from XUD to the Buffering cores */
    chan c_sof;

    /* Channel for audio data between buffering cores and AudioHub/IO core */
    chan c_aud;

    /* Channel for communicating control messages from EP0 to the rest of the
     * device (via the buffering cores) */
    chan c_aud_ctl;

    /* Channel for communicating between mic_array and audio */
    chan c_mic_pcm;

    /* par statement to run the following tasks in parallel */
    par
    {
        /* Low level USB device layer core */
        on tile[XUD_TILE]: XUD_Main(c_ep_out, 1, c_ep_in, 2,
                             c_sof, epTypeTableOut, epTypeTableIn,
                             XUD_SPEED_FS, XUD_PWR_BUS);

        /* Endpoint 0 core from lib_xua
         * Note: since we are not using many features we
         * pass in null for quite a few parameters */
        on tile[XUD_TILE]: XUA_Endpoint0(c_ep_out[0], c_ep_in[0], c_aud_ctl,
                                  null, null, null, null);

        /* Spin up the USB audio buffers. Note XUA_Buffer uses two cores.
         * If using a hardwired MCLK, COUNT_PORT will be p_for_mclk_count. 
         * Otherwise, it will be null. */
        on tile[XUD_TILE]: 
        {
            #if USE_APP_PLL_TILE_0
            /* Connect master-clock clock-block to clock-block pin */
            /* Clock clock-block from mclk pin */
            set_clock_src(clk_audio_mclk_usb, p_mclk_in_usb);           
            /* Clock the "count" port from the clock block */
            set_port_clock(COUNT_PORT, clk_audio_mclk_usb);       
            start_clock(clk_audio_mclk_usb); 
            #endif

            XUA_Buffer(c_ep_in[1], c_sof, c_aud_ctl, 
                                COUNT_PORT, c_aud);
        }

        /* AudioHub/IO core does most of the audio IO i.e. send to USB, and
         * also serves as a hub for all audio including mics */
        on tile[AUDIO_IO_TILE]: XUA_AudioHub(c_aud,
                                 null, null, null, null, null, null, null,
                                 c_mic_pcm);

        /* Microphone task */
        on tile[AUDIO_IO_TILE]:
        {
            /* Start the master clock */
            sw_pll_fixed_clock(MCLK_48);

            mic_array_task(c_mic_pcm);
        }
    }
    return 0;
}

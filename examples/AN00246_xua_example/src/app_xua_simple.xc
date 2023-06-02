// Copyright 2017-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* A very simple *example* of a USB audio application (and as such is un-verified for production)
 *
 * It uses the main blocks from the lib_xua
 *
 * - 2 channels out I2S only
 * - No DFU
 * - I2S only
 *
 */

#include <xs1.h>
#include <platform.h>

#include "xua.h"
#include "xud_device.h"

/* Port declarations. Note, the defines come from the xn file */
buffered out port:32 p_i2s_dac[]    = {PORT_I2S_DAC0};   /* I2S Data-line(s) */
buffered out port:32 p_lrclk        = PORT_I2S_LRCLK;    /* I2S Bit-clock */
buffered out port:32 p_bclk         = PORT_I2S_BCLK;     /* I2S L/R-clock */

/* Master clock for the audio IO tile */
in port p_mclk_in                   = PORT_MCLK_IN;

/* Resources for USB feedback */
in port p_for_mclk_count            = PORT_MCLK_COUNT;   /* Extra port for counting master clock ticks */
in port p_mclk_in_usb               = PORT_MCLK_IN_USB;  /* Extra master clock input for the USB tile */

/* Clock-block declarations */
clock clk_audio_bclk                = on tile[1]: XS1_CLKBLK_4;   /* Bit clock */
clock clk_audio_mclk                = on tile[1]: XS1_CLKBLK_5;   /* Master clock */
clock clk_audio_mclk_usb            = on tile[0]: XS1_CLKBLK_1;   /* Master clock for USB tile */

/* Endpoint type tables - informs XUD what the transfer types for each Endpoint in use and also
 * if the endpoint wishes to be informed of USB bus resets */
XUD_EpType epTypeTableOut[]   = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};
XUD_EpType epTypeTableIn[]    = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};

/* Port declarations for I2C to config ADC's */
on tile[0]: port p_scl = XS1_PORT_1L;
on tile[0]: port p_sda = XS1_PORT_1M;

/* See hwsupport.xc */
void ctrlPort();

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

    par
    {
        /* Low level USB device layer core */
        on tile[0]: XUD_Main(c_ep_out, 2, c_ep_in, 2,
                      c_sof, epTypeTableOut, epTypeTableIn,
                      XUD_SPEED_HS, XUD_PWR_SELF);

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

                        XUA_Buffer(c_ep_out[1], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);

                    }

        /* AudioHub/IO core does most of the audio IO i.e. I2S (also serves as a hub for all audio) */
        on tile[1]: XUA_AudioHub(c_aud, clk_audio_mclk, clk_audio_bclk, p_mclk_in, p_lrclk, p_bclk, p_i2s_dac, null);

        on tile[0]: ctrlPort();
    }

    return 0;
}



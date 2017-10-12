
/* A very simple example of a USB audio application (and as such is un-verified)
 * 
 * It uses the main blocks from the lib_xua 
 *
 */

#include <xs1.h>
#include <platform.h>

#include "xud_device.h"
#include "xua.h"

/* Ports - note the defines come from the xn file */

/* I2S ports - Data-line, bit-clock and L/R clock */
buffered out port:32 p_i2s_dac[]    = {PORT_I2S_DAC0};
buffered out port:32 p_lrclk        = PORT_I2S_LRCLK;
buffered out port:32 p_bclk         = PORT_I2S_BCLK;

/* Audio master-clock port */
port p_mclk_in                      = PORT_MCLK_IN;

/* Port for counting master clocks */
in port p_for_mclk_count            = PORT_MCLK_COUNT;

/* Clock-blocks for master-clock and bit-clock */
clock clk_audio_bclk                = on tile[0]: XS1_CLKBLK_4;       
clock clk_audio_mclk                = on tile[0]: XS1_CLKBLK_5;       

/* Endpoint type tables - informs XUD what the transfer types for each Endpoint in use and also
 * if the endpoint wishes to be informed of USB bus resets
 */
XUD_EpType epTypeTableOut[]   = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};
XUD_EpType epTypeTableIn[]    = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};

int main()
{
    /* Channels for lib_xud */
    chan c_ep_out[2];
    chan c_ep_in[2];

    /* TODO handle this */
    chan c_aud_ctl;

    chan c_sof;

    /* Channel for audio data between buffering cores and audio IO core */
    chan c_aud;

    par
    {
        /* Low level USB device layer core */ 
        on tile[1]: XUD_Main(c_ep_out, 2, c_ep_in, 2,
                      c_sof, epTypeTableOut, epTypeTableIn, 
                      /* TODO rm me! */
                      null, null, -1 , 
                      XUD_SPEED_HS, XUD_PWR_BUS);
        
        /* Endpoint 0 core from lib_xua */
        /* Note, since we are not using many features we pass in null for quite a few params.. */
        on tile[1]: XUA_Endpoint0(c_ep_out[0], c_ep_in[0], c_aud_ctl, null, null, null, null);

        /* Buffering cores - handles audio data to/from EP's and gives/gets data from the audio I/O core */
        /* Note, this spawns two cores */
        on tile[1]: XUA_Buffer(c_ep_out[1], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);


#if 0
        /* Audio I/o core i.e. I2S */
        on tile[1]: audio(AUDIO_CHANNEL,
#if defined(SPDIF_TX) && (SPDIF_TX_TILE != AUDIO_IO_TILE)
                c_spdif_tx,
#endif
#if defined(SPDIF_RX) || defined(ADAT_RX)
                c_dig_rx,
#endif
                c_aud_cfg, c_adc
#if (XUD_TILE != 0) && (AUDIO_IO_TILE == 0)
                , dfuInterface
#endif
#if (NUM_PDM_MICS > 0)
                , c_pdm_pcm
#endif
                , i_audMan
            );
#endif
        }
    
    return 0;
}



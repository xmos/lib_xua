
/* A very simple *example* of a USB audio application (and as such is un-verified for production)
 *
 * It uses the main blocks from the lib_xua 
 *
 * - 2 in/ 2 out I2S only
 * - No DFU
 * - I2S only 
 *
 */

#include <xs1.h>
#include <platform.h>

#include "xud_device.h"
#include "xua.h"

/* Port declarations. Note, the defines come from the xn file */
buffered out port:32 p_i2s_dac[]    = {PORT_I2S_DAC0};   /* I2S Data-line(s) */
buffered in port:32 p_i2s_adc[]    	= {PORT_I2S_ADC0};   /* I2S Data-line(s) */
buffered out port:32 p_lrclk        = PORT_I2S_LRCLK;    /* I2S Bit-clock */
buffered out port:32 p_bclk         = PORT_I2S_BCLK;     /* I2S L/R-clock */

/* Note, declared unsafe as sometimes we want to share this port
e.g. PDM mics and I2S use same master clock IO */
port p_mclk_in_          = PORT_MCLK_IN;

unsafe
{
    unsafe port p_mclk_in;                           /* Audio master clock input */
}

in port p_for_mclk_count            = PORT_MCLK_COUNT;   /* Extra port for counting master clock ticks */

/* Clock-block declarations */
clock clk_audio_bclk                = on tile[0]: XS1_CLKBLK_4;   /* Bit clock */    
clock clk_audio_mclk                = on tile[0]: XS1_CLKBLK_5;   /* Master clock */

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

    /* Channel for audio data between buffering cores and audio IO core */
    chan c_aud;
    
    /* Channel for communcating control messages from EP0 to the rest of the device (via the buffering cores */
    chan c_aud_ctl;

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

        /* Buffering cores - handles audio data to/from EP's and gives/gets data to/from the audio I/O core */
        /* Note, this spawns two cores */
        on tile[1]: XUA_Buffer(c_ep_out[1], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);

        /* IOHub core does most of the audio IO i.e. I2S (also serves as a hub for all audio) */
        on tile[0]: {
                        unsafe
                        {
                            p_mclk_in = p_mclk_in_; 
                        }
                        XUA_AudioHub(c_aud);
                    }
    }
    
    return 0;
}



// Copyright 2017-2025 XMOS LIMITED.
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
#include "xk_audio_316_mc_ab/board.h"

#undef I2S_CHANS_PER_FRAME  // avoid repeat define warning from I2S (defined in XUA)
#define I2S_DATA_BITS   32
#include "i2s.h"
#include "debug_print.h"

// #include "xua_audiohub_st.h"

/* Port declarations. Note, the defines come from the xn file */
buffered out port:32 p_i2s_dac[]    = {PORT_I2S_DAC0, PORT_I2S_DAC1};  /* I2S Data-line(s) */
buffered in port:32 p_i2s_adc[]     = {PORT_I2S_ADC0, PORT_I2S_ADC1};   /* I2S Data-line(s) */
#if I2S_MASTER
buffered out port:32 p_lrclk        = PORT_I2S_LRCLK;    /* I2S LR-clock */
out port p_bclk                     = PORT_I2S_BCLK;     /* I2S Bit-clock */
#else
buffered in port:32 p_lrclk         = PORT_I2S_LRCLK;    /* I2S LR-clock */
in port p_bclk                      = PORT_I2S_BCLK;     /* I2S Bit-clock */
#endif

/* Master clock for the audio IO tile */
in port p_mclk_in                   = PORT_MCLK_IN;

/* Resources for USB feedback */
in port p_for_mclk_count            = on tile[XUD_TILE]: XS1_PORT_16B;
in port p_mclk_in_usb               = PORT_MCLK_IN_USB;  /* Extra master clock input for the USB tile */

/* Clock-block declarations */
clock clk_audio_bclk                = on tile[1]: XS1_CLKBLK_2;   /* Bit clock */
clock clk_audio_mclk                = on tile[1]: XS1_CLKBLK_3;   /* Master clock */
clock clk_audio_mclk_usb            = on tile[XUD_TILE]: XS1_CLKBLK_1;   /* Master clock for USB tile */


/* Board configuration from lib_board_support */
static const xk_audio_316_mc_ab_config_t hw_config = {
        CLK_FIXED,              // clk_mode. Drive a fixed MCLK output
#if I2S_MASTER
        0,                      // 0 = dac_is_clock_slave
#else
        1,                      // 1 = dac_is_clock_master
#endif
        DEFAULT_MCLK,
        0,                      // pll_sync_freq (unused when driving fixed clock)
        AUD_316_PCM_FORMAT_I2S,
        I2S_DATA_BITS,          // data bits
        I2S_CHANS_PER_FRAME     // channels per frame
};

static const xk_audio_316_mc_ab_config_t hw_config_xmos_slave = {
        CLK_FIXED,              // clk_mode. Drive a fixed MCLK output
        1,                      // 1 = dac_is_clock_master
        DEFAULT_MCLK,
        0,                      // pll_sync_freq (unused when driving fixed clock)
        AUD_316_PCM_FORMAT_I2S,
        I2S_DATA_BITS,          // data bits
        I2S_CHANS_PER_FRAME     // channels per frame
};


unsafe client interface i2c_master_if i_i2c_client;

void AudioHwInit()
{
    unsafe {
        while(!(unsigned) i_i2c_client);
        xk_audio_316_mc_ab_AudioHwInit((client interface i2c_master_if)i_i2c_client, hw_config);
    }
}

void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    unsafe {
        xk_audio_316_mc_ab_AudioHwConfig((client interface i2c_master_if)i_i2c_client, hw_config, samFreq, mClk, dsdMode, sampRes_DAC, sampRes_ADC);
    }
}



#include <math.h>
#define HZ  100
#define NUM (MIN_FREQ / HZ)

[[distributable]]
void i2s_callback_handler(server i2s_frame_callback_if i_i2s, chanend c_aud) {

    int32_t samples_to_host[NUM_USB_CHAN_IN] = {0};
    int32_t samples_from_host[NUM_USB_CHAN_OUT] = {0};

    AudioHwInit();

    while (1) {
        select {
            case i_i2s.init(i2s_config_t &?i2s_config, tdm_config_t &?tdm_config):
                unsigned curSamRes_DAC, curSamRes_ADC, mClk, curSamFreq;
                XUA_wrapper_get_stream_format(&curSamFreq, &mClk, &curSamRes_DAC, &curSamRes_ADC);
                i2s_config.mode = I2S_MODE_I2S;
                i2s_config.mclk_bclk_ratio = mClk / (curSamFreq * I2S_CHANS_PER_FRAME * I2S_DATA_BITS);
                i2s_config.slave_bclk_polarity == I2S_SLAVE_SAMPLE_ON_BCLK_RISING;
                AudioHwConfig(curSamFreq, mClk, 0, curSamRes_DAC, curSamRes_ADC);
                debug_printf("Sample rate: %u mClk: %u\n", curSamFreq, mClk);
                break;

            case i_i2s.restart_check() -> i2s_restart_t restart:
                int format_change = XUA_wrapper_exchange_samples(c_aud, samples_to_host, samples_from_host);
                // int format_change = 0;
                if(format_change){
                    restart = I2S_RESTART;
                } else {
                    restart = I2S_NO_RESTART;
                }
                static int sc=0; if(sc++ == 48000){printstrln("r");sc=0;} // print looping 1 per second
                break;

            case i_i2s.receive(size_t num_in, int32_t samples[num_in]):
                // Handle a received sample
                for(int i = 0; i < num_in; i++){
                    samples_to_host[i] = samples[i]; // copy to USB inbound buffer
                }
                break;

            case i_i2s.send(size_t num_out, int32_t samples[num_out]):
                // Provide a sample to send to I2S output
                for(int i = 0; i < num_out; i++){
                    samples[i] = samples_from_host[i];
                }
                break;
        }
    }
}


int main()
{
    /* Channel for audio data between buffering cores and AudioHub/IO core */
    chan c_aud;


    /* Interface for access to I2C for setting up hardware */
    interface i2c_master_if i2c[1];
    par {

        /* Buffering cores - handles audio data to/from EP's and gives/gets data to/from the audio I/O core */
        /* Note, this spawns two cores */
        on tile[XUD_TILE]: {
                        XUA_wrapper_task(c_aud);
                    }

        /* AudioHub/IO core does most of the audio IO i.e. I2S (also serves as a hub for all audio) */
        on tile[1]: {
                        unsafe {
                            i_i2c_client = i2c[0];
                        }
                        i2s_frame_callback_if i_i2s;
                        par
                        {
#if I2S_MASTER
                            i2s_frame_master(i_i2s, p_i2s_dac, NUM_USB_CHAN_OUT / 2, p_i2s_adc, NUM_USB_CHAN_IN / 2, I2S_DATA_BITS, p_bclk, p_lrclk, p_mclk_in, clk_audio_bclk);
#else
                            i2s_frame_slave(i_i2s, p_i2s_dac, NUM_USB_CHAN_OUT / 2, p_i2s_adc, NUM_USB_CHAN_IN / 2, I2S_DATA_BITS, p_bclk, p_lrclk, clk_audio_bclk);
#endif
                            [[distribute]]
                            i2s_callback_handler(i_i2s, c_aud);
                        }
                    }

        on tile[0]: {
                        xk_audio_316_mc_ab_board_setup(hw_config);
                        xk_audio_316_mc_ab_i2c_master(i2c);
                    }
    }

    return 0;
}

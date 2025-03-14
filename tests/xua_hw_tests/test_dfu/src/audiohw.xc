// Copyright 2024-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xua.h"
#include <xk_audio_316_mc_ab/board.h>

#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
    #define PLL_SYNC_FREQ           (500)
#else
    #if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
    /* Choose a frequency the xcore can easily generate internally */
    #define PLL_SYNC_FREQ           (300)
    #else
    #define PLL_SYNC_FREQ           (1000000)
    #endif
#endif

static xk_audio_316_mc_ab_config_t config = {
    (XUA_SYNCMODE == XUA_SYNCMODE_SYNC || XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
    ? ( XUA_USE_SW_PLL
        ? CLK_PLL : CLK_CS2100 )
    : CLK_FIXED,          // clk_mode
    CODEC_MASTER,         // dac_is_clk_master
    MCLK_48,              // default_mclk
    PLL_SYNC_FREQ,        // pll_sync_freq
    XUA_PCM_FORMAT,       // pcm_format
    XUA_I2S_N_BITS,       // i2s_n_bits
    I2S_CHANS_PER_FRAME,  // i2s_chans_per_frame
};

unsafe client interface i2c_master_if i_i2c_client;

void board_setup()
{
    xk_audio_316_mc_ab_board_setup(config);
}

void AudioHwInit()
{
    unsafe{
        /* Wait until global is set */
        while(!(unsigned) i_i2c_client);
        xk_audio_316_mc_ab_AudioHwInit((client interface i2c_master_if)i_i2c_client, config);
    }
}

void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    unsafe {xk_audio_316_mc_ab_AudioHwConfig((client interface i2c_master_if)i_i2c_client, config, samFreq, mClk, dsdMode, sampRes_DAC, sampRes_ADC);}
}

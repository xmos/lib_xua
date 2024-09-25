// Copyright 2011-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <platform.h>
#include <print.h>
#include "xua.h"
#include "audioports.h"

extern clock    clk_audio_mclk;

void ConfigAudioPorts(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
        buffered out port:32 p_i2s_dac[],
        int numPortsDac,
#endif

#if (I2S_CHANS_ADC != 0)
        buffered in port:32  p_i2s_adc[],
        int numPortsAdc,
#endif

#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
#if (CODEC_MASTER == 0)
        buffered out port:32 ?p_lrclk,
        buffered out port:32 p_bclk,
#else
        in port ?p_lrclk,
        in port p_bclk,
#endif
#endif
        in port p_mclk_in, clock clk_audio_bclk, unsigned int divide, unsigned curSamFreq)
{
#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)

#if (CODEC_MASTER == 0)
#ifdef __XS3A__
  	/* Increase drive strength of clock ports to 8mA */
	asm volatile ("setc res[%0], %1" :: "r" (p_bclk), "r" (0x200006));
	asm volatile ("setc res[%0], %1" :: "r" (p_lrclk), "r" (0x200006));
#endif

    /* Note this call to stop_clock() will pause forever if the port clocking the clock-block is not low.
     * deliver() should return with this being the case */
    stop_clock(clk_audio_bclk);

    if(!isnull(p_lrclk))
    {
        clearbuf(p_lrclk);
    }
    clearbuf(p_bclk);

#if (I2S_CHANS_ADC != 0)
    for(int i = 0; i < numPortsAdc; i++)
    {
        clearbuf(p_i2s_adc[i]);
    }
#endif

#if (I2S_CHANS_DAC != 0)|| (DSD_CHANS_DAC != 0)
    for(int i = 0; i < numPortsDac; i++)
    {
        clearbuf(p_i2s_dac[i]);
    }
#endif

    unsafe
    {
        /* Clock bitclock clock block from master clock pin (divided) */
        configure_clock_src_divide(clk_audio_bclk, (port) p_mclk_in, (divide/2));
        configure_port_clock_output(p_bclk, clk_audio_bclk);
    }

    if(!isnull(p_lrclk))
    {
        /* Clock LR clock from bit clock-block */
        configure_out_port_no_ready(p_lrclk, clk_audio_bclk, 0);
    }

#if (I2S_CHANS_ADC != 0)
    /* Some adustments for timing. Sample ADC lines on negative edge and add some delay */
    if(XUA_PCM_FORMAT == XUA_PCM_FORMAT_TDM)
    {
        for(int i = 0; i < numPortsAdc; i++)
        {
            set_port_sample_delay(p_i2s_adc[i]);
            set_pad_delay(p_i2s_adc[i], 4);
        }
    }
#endif

#elif (CODEC_MASTER)
    /* Stop bit and master clock blocks */
    stop_clock(clk_audio_bclk);

    /* Clock bclk clock-block from bclk pin */
    configure_clock_src(clk_audio_bclk, p_bclk);

    configure_in_port_no_ready(p_lrclk, clk_audio_bclk);

 	/* Do some clocking shifting to get data in the valid window */
    /* E.g. Only shift when running at 88.2+ kHz TDM slave */
    int bClkDelay_fall = 0;
    if(curSamFreq * I2S_CHANS_PER_FRAME * XUA_I2S_N_BITS >= 20000000)
    {
        /* 18 * 2ns = 36ns. This results in a -4ns (36 - 40) shift at 96KHz and -8ns (36 - 44) at 88.4KHz */
        bClkDelay_fall = 18;
    }

    set_clock_fall_delay(clk_audio_bclk, bClkDelay_fall);
#endif

#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
     /* Clock I2S/DSD output data ports from b-clock clock block */
    for(int i = 0; i < numPortsDac; i++)
    {
        configure_out_port_no_ready(p_i2s_dac[i], clk_audio_bclk, 0);
    }
#endif

#if (I2S_CHANS_ADC != 0)
    /* Clock I2S input data ports from clock block */
    for(int i = 0; i < numPortsAdc; i++)
    {
        configure_in_port_no_ready(p_i2s_adc[i], clk_audio_bclk);
    }
#endif

    /* Start clock blocks ticking */
    start_clock(clk_audio_bclk);

#endif //#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
}


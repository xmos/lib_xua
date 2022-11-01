// Copyright 2018-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xua.h"

#include "dsd_support.h"

#if (DSD_CHANS_DAC != 0)
extern buffered out port:32 p_dsd_dac[DSD_CHANS_DAC];
extern buffered out port:32 p_dsd_clk;
#endif

extern unsigned dsdMode;

#if !CODEC_MASTER
void InitPorts_master(unsigned divide, buffered _XUA_CLK_DIR port:32 p_lrclk, buffered _XUA_CLK_DIR port:32 p_bclk, buffered out port:32 (&?p_i2s_dac)[I2S_WIRES_DAC], buffered in port:32  (&?p_i2s_adc)[I2S_WIRES_ADC])
{
#if (DSD_CHANS_DAC > 0)
    if(dsdMode == DSD_MODE_OFF)
    {
#endif

#if (I2S_CHANS_ADC != 0 || I2S_CHANS_DAC != 0)

        /* Clear I2S port buffers */
        clearbuf(p_lrclk);

#if (I2S_CHANS_DAC != 0)
        for(int i = 0; i < I2S_WIRES_DAC; i++)
        {
            clearbuf(p_i2s_dac[i]);
        }
#endif

#if (I2S_CHANS_ADC != 0)
        for(int i = 0; i < I2S_WIRES_ADC; i++)
        {
            clearbuf(p_i2s_adc[i]);
        }
#endif

        unsigned tmp;
        p_lrclk <: 0 @ tmp;
        tmp += 100;

        /* Since BCLK is free-running, setup outputs/inputs at a known point in the future */
#if (I2S_CHANS_DAC != 0)
#pragma loop unroll
        for(int i = 0; i < I2S_WIRES_DAC; i++)
        {
            p_i2s_dac[i] @ tmp <: 0;
        }
#endif

        if(XUA_PCM_FORMAT == XUA_PCM_FORMAT_TDM)
            p_lrclk @ tmp <: 0x80000000;
        else
            p_lrclk @ tmp <: 0x7FFFFFFF;

#if (I2S_CHANS_ADC != 0)
        for(int i = 0; i < I2S_WIRES_ADC; i++)
        {
            asm("setpt res[%0], %1"::"r"(p_i2s_adc[i]),"r"(tmp-1));
        }
#endif
#endif /* (I2S_CHANS_ADC != 0 || I2S_CHANS_DAC != 0) */


#if (DSD_CHANS_DAC > 0)
    } /* if (!dsdMode) */
    else
    {
        /* p_dsd_clk must start high */
        p_dsd_clk <: 0x80000000;
    }
#endif
}
#else
void InitPorts_slave(unsigned divide, buffered _XUA_CLK_DIR port:32 p_lrclk, buffered _XUA_CLK_DIR port:32 p_bclk, buffered out port:32 (&?p_i2s_dac)[I2S_WIRES_DAC], buffered in port:32  (&?p_i2s_adc)[I2S_WIRES_ADC])
{
#if (I2S_CHANS_ADC != 0 || I2S_CHANS_DAC != 0)
    unsigned tmp;

    /* Wait for LRCLK edge (in I2S LRCLK = 0 is left, TDM rising edge is start of frame) */
    p_lrclk when pinseq(0) :> void;
    p_lrclk when pinseq(1) :> void;
    p_lrclk when pinseq(0) :> void;
    p_lrclk when pinseq(1) :> void;
#if (XUA_PCM_FORMAT == XUA_PCM_FORMAT_TDM)
    p_lrclk when pinseq(0) :> void;
    p_lrclk when pinseq(1) :> void @ tmp;
#else
    p_lrclk when pinseq(0) :> void @ tmp;
#endif

    tmp += (I2S_CHANS_PER_FRAME * 32) - 32 + 1 ;
    /* E.g. 2 * 32 - 32 + 1 = 33 for stereo */
    /* E.g. 8 * 32 - 32 + 1 = 225 for 8 chan TDM */

#if (I2S_CHANS_DAC != 0)
#pragma loop unroll
    for(int i = 0; i < I2S_WIRES_DAC; i++)
    {
        p_i2s_dac[i] @ tmp <: 0;
    }
#endif

#if (I2S_CHANS_ADC != 0)
#pragma loop unroll
    for(int i = 0; i < I2S_WIRES_ADC; i++)
    {
       asm("setpt res[%0], %1"::"r"(p_i2s_adc[i]),"r"(tmp-1));
    }
#endif

    asm("setpt res[%0], %1"::"r"(p_lrclk),"r"(tmp-1));
#endif /* (I2S_CHANS_ADC != 0 || I2S_CHANS_DAC != 0) */
}
#endif


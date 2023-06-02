// Copyright 2013-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xs1.h>
#define __ASSEMBLER__ // Work around for bug #14118
#include <platform.h>
#undef __ASSEMBLER__
#include "audioports.h"
#include "xua.h"

/* Note since DSD ports could be reused for I2S ports we do all the setup manually in C */
#if (DSD_CHANS_DAC > 0)
port p_dsd_dac[DSD_CHANS_DAC] = {
                PORT_DSD_DAC0,
#endif
#if (DSD_CHANS_DAC > 1)
                PORT_DSD_DAC1,
#endif
#if (DSD_CHANS_DAC > 2)
#error > 2 DSD chans currently not supported
#endif
#if (DSD_CHANS_DAC > 0)
                };
port p_dsd_clk = PORT_DSD_CLK;
#endif

void EnableBufferedPort(port p, unsigned transferWidth)
{
    asm volatile("setc res[%0], %1"::"r"(p), "r"(XS1_SETC_INUSE_ON));
    asm volatile("setc res[%0], %1"::"r"(p), "r"(XS1_SETC_BUF_BUFFERS));
    asm volatile("settw res[%0], %1"::"r"(p),"r"(transferWidth));
}

/* C wrapper for ConfigAudioPorts() to handle DSD ports */
void ConfigAudioPortsWrapper(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
                port p_dac[], int numPortsDac,
#endif

#if (I2S_CHANS_ADC != 0)
                port p_adc[], int numPortsAdc,
#endif

#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
                port p_lrclk,
                port p_bclk,
#endif
                port p_mclk_in, clock clk_audio_bclk, unsigned int divide, unsigned curSamFreq)
{
        ConfigAudioPorts(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
                p_dac,
                numPortsDac,
#endif
#if (I2S_CHANS_ADC != 0)
                p_adc,
                numPortsAdc,
#endif
#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
                p_lrclk,
                p_bclk,
#endif
                p_mclk_in, clk_audio_bclk, divide, curSamFreq);
}


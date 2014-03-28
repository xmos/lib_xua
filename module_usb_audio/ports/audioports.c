
#include <xs1.h>
#define __ASSEMBLER__ // Work around for bug #14118
#include <platform.h>
#undef __ASSEMBLER__
#include "devicedefines.h"
#include "audioports.h"

#if (DSD_CHANS_DAC != 0)
/* Note since DSD ports could be reused for I2S ports we do all the setup manually in C */
#if DSD_CHANS_DAC > 0
port p_dsd_dac[DSD_CHANS_DAC] = {
                PORT_DSD_DAC0,
#endif
#if DSD_CHANS_DAC > 1
                PORT_DSD_DAC1,
#endif
#if DSD_CHANS_DAC > 2
#error > 2 DSD chans currently not supported
#endif
#if DSD_CHANS_DAC > 0
                };
port p_dsd_clk = PORT_DSD_CLK;
#endif

static inline void EnableBufferedPort(port p, unsigned transferWidth)
{
    //set_port_use_on(p_dsd_dac[i]);
    asm volatile("setc res[%0], %1"::"r"(p), "r"(XS1_SETC_INUSE_ON));
    asm volatile("setc res[%0], %1"::"r"(p), "r"(XS1_SETC_BUF_BUFFERS));
    asm volatile("settw res[%0], %1"::"r"(p),"r"(transferWidth));
}


/* C wrapper for ConfigAudioPorts() such that we can mess around with arrays of ports */
void ConfigAudioPortsWrapper(
#if (I2S_CHANS_DAC != 0)
                port p_i2s_dac[I2S_WIRES_DAC],
#endif

#if (I2S_CHANS_ADC != 0)
                port p_i2s_adc[I2S_WIRES_ADC],
#endif

#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
#ifndef CODEC_MASTER
                port p_lrclk,
                port p_bclk,
#else
                in port p_lrclk,
                in port p_bclk,
#endif
#endif
unsigned int divide, unsigned int dsdMode)
{
    /* Ensure dsd clock is on in all modes since I2S mode sets it low on exit
     * to avoid stop_clock() potentially pausing forever. If this is not done
     * an exception will be raised with audio() attempts to set this port low
     */
    /* TODO Do we really need to do this on every SF change? Once is probably enough */
    EnableBufferedPort(p_dsd_clk, 32);

    if(dsdMode)
    {
        /* Make sure the ports are on and buffered - just in case they are not shared with I2S */
        for(int i = 0; i< DSD_CHANS_DAC; i++)
        {
            EnableBufferedPort(p_dsd_dac[i], 32);
        }

        ConfigAudioPorts(
#if (DSD_CHANS_DAC != 0)
                p_dsd_dac,
                DSD_CHANS_DAC,
#endif
#if (I2S_CHANS_ADC != 0)
                p_i2s_adc,
                I2S_WIRES_ADC,
#endif
#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
#ifndef CODEC_MASTER
                0, /* NULL */
                p_dsd_clk,
#else
                0, /* NULL */
                p_dsd_clk,
#endif
#endif
                divide);

    }
    else
    {
        ConfigAudioPorts(
#if (I2S_CHANS_DAC != 0)
                p_i2s_dac,
                I2S_WIRES_DAC,
#endif
#if (I2S_CHANS_ADC != 0)
                p_i2s_adc,
                I2S_WIRES_ADC,
#endif
#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
#ifndef CODEC_MASTER
                p_lrclk,
                p_bclk,
#else
                p_lrclk,
                p_bclk,
#endif
#endif
                divide);
    }
}
#endif

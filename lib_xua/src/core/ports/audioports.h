// Copyright 2011-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _AUDIOPORTS_H_
#define _AUDIOPORTS_H_

#include <xccompat.h>


#include "xua.h"

void ConfigAudioPorts(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
    out_buffered_port_32_t p_i2s_dac[],
    int numDacPorts,
#endif
#if (I2S_CHANS_ADC != 0)
    in_buffered_port_32_t p_i2s_adc[],
    int numAdcPorts,
#endif
#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
#if (CODEC_MASTER == 0)
    NULLABLE_RESOURCE(out_buffered_port_32_t, p_lrclk),
    out_buffered_port_32_t p_bclk,
#else
    NULLABLE_RESOURCE(in_port_t, p_lrclk),
    in_port_t p_bclk,
#endif
#endif
#if (XUA_I2S_EN)
    NULLABLE_RESOURCE(in_port_t,  p_mclk_in),
#endif
    clock clk_audio_bclk, unsigned int divide, unsigned int curSamFreq);
//#else    // Delete???


void ConfigAudioPortsWrapper(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
    out_buffered_port_32_t p_i2s_dac[],
    int numPortsDAC,
#endif
#if (I2S_CHANS_ADC != 0)
    in_buffered_port_32_t  p_i2s_adc[],
    int numPortsADC,
#endif
#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
#if (CODEC_MASTER == 0)
    NULLABLE_RESOURCE(out_buffered_port_32_t, p_lrclk),
    out_buffered_port_32_t p_bclk,
#else
    NULLABLE_RESOURCE(in_buffered_port_32_t, p_lrclk),
    in_buffered_port_32_t p_bclk,
#endif
#endif
#if (XUA_I2S_EN)
    NULLABLE_RESOURCE(in_port_t,  p_mclk_in),
#endif
    clock clk_audio_bclk, unsigned int divide, unsigned curSamFreq);

#ifdef __XC__
void EnableBufferedPort(buffered out port:32 p, unsigned transferWidth);
#else
void EnableBufferedPort(port p, unsigned transferWidth);
#endif

#endif /* _AUDIOPORTS_H_ */

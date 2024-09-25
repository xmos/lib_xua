// Copyright 2011-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _AUDIOPORTS_H_
#define _AUDIOPORTS_H_

#include <xccompat.h>
#ifdef __STDC__
typedef unsigned clock;
#endif
#include "xua.h"

#ifdef __XC__
void ConfigAudioPorts(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
                buffered out port:32 p_i2s_dac[],
                int numDacPorts,

#endif

#if (I2S_CHANS_ADC != 0)
                buffered in port:32  p_i2s_adc[],
                int numAdcPorts,
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
                in port p_mclk_in, clock clk_audio_bclk, unsigned int divide, unsigned int curSamFreq);
#else

void ConfigAudioPorts(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
                port p_i2s_dac[],
                int numDacPorts,
#endif

#if (I2S_CHANS_ADC != 0)
                port p_i2s_adc[],
                int numAdcPorts,
#endif

#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
#if (CODEC_MASTER == 0)
                port p_lrclk,
                port p_bclk,
#else
                port p_lrclk,
                port p_bclk,
#endif
#endif
                port p_mclk_in, clock clk_audio_bclk, unsigned int divide, unsigned int curSamFreq);


#endif /* __XC__*/


#ifdef __XC__
void ConfigAudioPortsWrapper(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
                buffered out port:32 p_i2s_dac[], int numPortsDAC,
#endif

#if (I2S_CHANS_ADC != 0)
                buffered in port:32  p_i2s_adc[], int numPortsADC,
#endif

#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
#if (CODEC_MASTER == 0)
                buffered out port:32 ?p_lrclk,
                buffered out port:32 p_bclk,
#else
                buffered in port:32 ?p_lrclk,
                buffered in port:32 p_bclk,
#endif
#endif
                in port p_mclk_in, clock clk_audio_bclk, unsigned int divide, unsigned curSamFreq);
#else

void ConfigAudioPortsWrapper(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
                port p_i2s_dac[], int numPortsDAC,
#endif

#if (I2S_CHANS_ADC != 0)
                port p_i2s_adc[], int numPortsADC,
#endif

#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
                port p_lrclk,
                port p_bclk,
#endif
                port p_mclk_in, clock clk_audio_bclk, unsigned int divide, unsigned curSamFreq);


#endif /* __XC__*/

#ifdef __XC__
void EnableBufferedPort(buffered out port:32 p, unsigned transferWidth);
#else
void EnableBufferedPort(port p, unsigned transferWidth);
#endif

#endif /* _AUDIOPORTS_H_ */

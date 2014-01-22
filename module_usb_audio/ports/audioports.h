#ifndef _AUDIOPORTS_H_
#define _AUDIOPORTS_H_

#include <xccompat.h>
#include "devicedefines.h"

void ConfigAudioPorts_dsd(unsigned int divide);


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
#ifndef CODEC_MASTER
                buffered out port:32 ?p_lrclk,
                buffered out port:32 p_bclk,
#else
                in port ?p_lrclk,
                in port p_bclk,
#endif
#endif
                unsigned int divide);
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
#ifndef CODEC_MASTER
                port p_lrclk,
                port p_bclk,
#else
                port p_lrclk,
                port p_bclk,
#endif
#endif
                unsigned int divide);


#endif /* __XC__*/


#ifdef __XC__
void ConfigAudioPortsWrapper(
#if (I2S_CHANS_DAC != 0)
                buffered out port:32 p_i2s_dac[I2S_WIRES_DAC],
#endif

#if (I2S_CHANS_ADC != 0)
                buffered in port:32  p_i2s_adc[I2S_WIRES_ADC],
#endif

#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
#ifndef CODEC_MASTER
                buffered out port:32 p_lrclk,
                buffered out port:32 p_bclk,
#else
                in port p_lrclk,
                in port p_bclk,
#endif
#endif
                unsigned int divide, unsigned int dsdMode);
#else

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
                port p_lrclk,
                port p_bclk,
#endif
#endif
                unsigned int divide, unsigned int dsdMode);


#endif /* __XC__*/




#endif /* _AUDIOPORTS_H_ */

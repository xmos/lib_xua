
#include <xccompat.h>
#include "devicedefines.h"
#include "audioports.h"

//#define p_dsd_left   p_i2s_dac[0]
//#define p_dsd_right  p_bclk
#define p_dsd_clk    p_lrclk
//#d
#if 0
#ifndef p_dsd_clk
buffered out port:32 p_dsd_clk = P_DSD_CLK;
#endif

#ifndef p_dsd_left
extern buffered out port:32 p_dsd_left;
#endif

#ifndef p_dsd_right
extern buffered out port:32 p_dsd_right;
#endif

#if I2S_WIRES_DAC > 0
#ifndef p_dsd_dac0
on tile[0] : buffered out port:32 p_dsd_dac0 = PORT_DSD_DAC0; 
#endif
dsdPorts[0] = PORT_DSD_DAC0;
#endif 

#endif

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

   if(dsdMode)
    {
    
    }
    else
    {
        ConfigAudioPorts(
#if (I2S_CHANS_DAC != 0)
                p_i2s_dac,
#endif
#if (I2S_CHANS_ADC != 0)
                p_i2s_adc,
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

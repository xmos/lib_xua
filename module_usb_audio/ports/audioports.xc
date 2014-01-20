#include <xs1.h>
#include <platform.h>
#include <print.h>
#include "devicedefines.h"
#include "audioports.h"

extern port p_mclk_in;
extern clock    clk_audio_mclk;
extern clock    clk_audio_bclk;

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
#ifndef CODEC_MASTER
            buffered out port:32 ?p_lrclk,
            buffered out port:32 p_bclk,
#else
            in port ?p_lrclk,
            in port p_bclk,
#endif
#endif
unsigned int divide) 
{

#ifndef CODEC_MASTER
#if 0
    {
        unsigned clockResId;
        unsigned portResId;
        asm("add %0, %1, %2": "=r"(clockResId):"r"(clk_audio_bclk), "r"(0));
        portResId = getps(clockResId | (XS1_RES_PS_CLKSRC << 4) );
        if(portResId !=1)
            asm("out res[%0], %1":: "r"(portResId), "r"(0));
    }
#endif

    /* Note this call to stop_clock() will pause forever if the port clocking the clock-block is not low.
     * deliver() should return with this being the case - however, if you are having trouble the code 
     * above can be enabled */
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

#if (I2S_CHANS_DAC != 0)
    for(int i = 0; i < numPortsDac; i++)
    {
        clearbuf(p_i2s_dac[i]);
    }
#endif

    /* Clock master clock-block from master-clock port */
    // Now done outside of function
    //configure_clock_src(clk_audio_mclk, p_mclk_in);

    /* For a divide of one (i.e. bitclock == master-clock) BClk is set to clock_output mode.
     * In this mode it outputs an edge clock on every tick of itsassociated clock_block.
     *
     * For all other divides, BClk is clocked by the master clock and data
     * will be output to p_bclk to generate the bit clock.
     */
    if (divide == 1) /* e.g. 176.4KHz from 11.2896 */
    {
        configure_port_clock_output(p_bclk, clk_audio_mclk);
    }
    else
    {
        /* bit clock port from master clock clock-clock block */
        configure_out_port_no_ready(p_bclk, clk_audio_mclk, 0);
    }

    /* Generate bit clock block from pin */
    configure_clock_src(clk_audio_bclk, p_bclk);

    if(!isnull(p_lrclk))
    {
        /* Clock LR clock from bit clock-block */
        configure_out_port_no_ready(p_lrclk, clk_audio_bclk, 0);
    }

#if (I2S_CHANS_DAC != 0)
    /* Clock I2S output data ports from clock block */
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

#else /* CODEC_MASTER */

    /* Stop bit and master clock blocks */
    stop_clock(clk_audio_bclk);
    //stop_clock(clk_audio_mclk);

    /* Clock master clock-block from master-clock port - 
     * though not directly used in I2S slave mode it is required for FB */
    //configure_clock_src(clk_audio_mclk, p_mclk_in);
    
    /* Clock bclk clock-block from bclk pin */
    configure_clock_src(clk_audio_bclk, p_bclk);

     /* Clock I2S output data ports from b-clock clock block */
    for(int i = 0; i < I2S_WIRES_DAC; i++)
    {
        configure_out_port_no_ready(p_i2s_dac[i], clk_audio_bclk, 0);
    }

    /* Clock I2S input data ports from clock block */
    for(int i = 0; i < I2S_WIRES_ADC; i++)
    {
        configure_in_port_no_ready(p_i2s_adc[i], clk_audio_bclk);
    }

    configure_in_port_no_ready(p_lrclk, clk_audio_bclk);

    start_clock(clk_audio_bclk);
    //start_clock(clk_audio_mclk);

#endif
}

#if 0
void ConfigAudioPorts_dsd(unsigned int divide) 
{

#ifndef CODEC_MASTER
    /* Output 0 on BCLK to ensure clock is low
     * Required as stop_clock will only complete when the clock is low
     */
    //configure_out_port_no_ready(p_dsd_clk, clk_audio_bclk, 0);
    //configure_clock_src(clk_audio_mclk, p_mclk_in);
    configure_out_port_no_ready(p_dsd_clk, clk_audio_mclk, 0);
    p_dsd_clk <: 0;

    /* Stop bit and master clock blocks and clear port buffers */
    stop_clock(clk_audio_bclk);
    stop_clock(clk_audio_mclk);

    clearbuf(p_dsd_clk);
    clearbuf(p_dsd_left);
    clearbuf(p_dsd_right);

    /* Clock master clock-block from master-clock port */
    configure_clock_src(clk_audio_mclk, p_mclk_in);

    /* For a divide of one (i.e. bitclock == master-clock) BClk is set to clock_output mode.
     * In this mode it outputs an edge clock on every tick of itsassociated clock_block.
     *
     * For all other divides, BClk is clocked by the master clock and data
     * will be output to p_bclk to generate the bit clock.
     */
    if (divide == 1) /* e.g. 176.4KHz from 11.2896 */
    {
        configure_port_clock_output(p_dsd_clk, clk_audio_mclk);
    }
    else
    {
        /* bit clock port from master clock clock-clock block */
        configure_out_port_no_ready(p_dsd_clk, clk_audio_mclk, 0);
    }

    /* bclk clock-blocked clocked by dsd_clk pin */
    configure_clock_src(clk_audio_bclk, p_dsd_clk);

    
    configure_out_port_no_ready(p_dsd_left, clk_audio_bclk, 0);
    configure_out_port_no_ready(p_dsd_right, clk_audio_bclk, 0);

    /* Start clock blocks ticking */
    start_clock(clk_audio_mclk);
    start_clock(clk_audio_bclk);

    /* bclk initial state needs to be high  */
    p_dsd_clk<: 0xFFFFFFFF;

    /* Pause until output completes */
    sync(p_dsd_clk);

#else /* CODEC_MASTER */
#error CODEC MASTER for DSD not currently implemented
#endif
}
#endif

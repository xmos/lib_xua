/**
 * @file audio.xc
 * @brief XMOS L1/L2 USB 2,0 Audio Reference Design.  Audio Functions.
 * @author Ross Owen, XMOS Semiconductor Ltd 
 *
 * This thread handles I2S and pars an additional SPDIF Tx thread.  It forwards samples to the SPDIF Tx thread.
 * Additionally this thread handles clocking and CODEC/DAC/ADC config.
 **/                               

#include <syscall.h>
#include <platform.h>
#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include <xs1_su.h>

#include "clocking.h"
#include "audioports.h"
#include "codec.h"
#include "devicedefines.h"
#include "SpdifTransmit.h"

unsigned g_adcVal = 0;

//#define RAMP_CHECK 1

//#pragma xta command "analyse path i2s_output_l i2s_output_r"
//#pragma xta command "set required - 2000 ns"

//#pragma xta command "analyse path i2s_output_r i2s_output_l"
//#pragma xta command "set required - 2000 ns"

/* I2S Data I/O*/
#if (I2S_CHANS_DAC != 0)
extern buffered out port:32 p_i2s_dac[I2S_WIRES_DAC];  
#endif

#if (I2S_CHANS_ADC != 0)
extern buffered in port:32  p_i2s_adc[I2S_WIRES_ADC];
#endif

/* I2S LR/Bit clock I/O */
#ifndef CODEC_MASTER
extern buffered out port:32 p_lrclk;
extern buffered out port:32 p_bclk;
#else
extern in port p_lrclk;
extern in port p_bclk;
#endif

/* Master clock input */
extern port p_mclk;

#ifdef SPDIF 
extern buffered out port:32 p_spdif_tx;
extern clock    clk_mst_spd;  
#endif

extern clock    clk_audio_mclk;  
extern clock    clk_audio_bclk;  

extern void device_reboot(void);

/* I2S delivery thread */
#pragma unsafe arrays
unsigned deliver(chanend c_out, chanend ?c_spd_out, unsigned divide, chanend ?c_dig_rx, chanend ?c_adc)
{
	unsigned sample;
#if NUM_USB_CHAN_OUT > 0 
    unsigned samplesOut[NUM_USB_CHAN_OUT];
#endif
#if NUM_USB_CHAN_IN > 0 
    unsigned samplesIn[NUM_USB_CHAN_IN];
    unsigned samplesInPrev[NUM_USB_CHAN_IN];
#endif
    unsigned tmp;
#if (I2S_CHANS_ADC != 0)
    unsigned index;
#endif
#ifdef RAMP_CHECK
    unsigned prev=0;
    int started = 0;
#endif


#if NUM_USB_CHAN_IN > 0
    for (int i=0;i<NUM_USB_CHAN_IN;i++)
    {
        samplesIn[i] = 0;
        samplesInPrev[i] = 0;
    }
#endif
    outuint(c_out, 0);

    /* Check for sample freq change or new samples from mixer*/
    if(testct(c_out))
    {
        inct(c_out);
        return inuint(c_out);

    }
    else
    {
#ifndef MIXER // Interfaces straight to decouple()
        (void) inuint(c_out);

#if NUM_USB_CHAN_IN > 0
#pragma loop unroll
        for(int i = 0; i < NUM_USB_CHAN_IN; i++)
        {
            outuint(c_out, samplesIn[i]);
        }
#endif

#if NUM_USB_CHAN_OUT > 0
#pragma loop unroll
        for(int i = 0; i < NUM_USB_CHAN_OUT; i++)        
        {
            samplesOut[i] = inuint(c_out);
        }
#endif
#else
#pragma loop unroll
        for(int i = 0; i < NUM_USB_CHAN_OUT; i++)        
        {
            int tmp = inuint(c_out);
#if defined(OUT_VOLUME_IN_MIXER) && defined(OUT_VOLUME_AFTER_MIX)
            tmp<<=3;
#endif
            samplesOut[i] = tmp;
        }
        	 
#pragma loop unroll
        for(int i = 0; i < NUM_USB_CHAN_IN; i++)
        {
            outuint(c_out, samplesIn[i]);
        }
#endif
    }

#ifndef CODEC_MASTER
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

    if(divide == 1)
    {
        p_lrclk <: 0 @ tmp;
        tmp += 30;
  
        /* Prefill the ports so data starts to be input */
#if (I2S_CHANS_DAC != 0)
#pragma loop unroll
        for(int i = 0; i < I2S_WIRES_DAC; i++)
        {
            p_i2s_dac[i] @ tmp <: 0;
        }
#endif

        p_lrclk @ tmp <: 0x7FFFFFFF;  


#if (I2S_CHANS_ADC != 0)
        p_i2s_adc[0]  @ (tmp - 1) :> void;  
#endif


#if (I2S_CHANS_ADC != 0)
#pragma loop unroll 
        for(int i = 0; i < I2S_WIRES_ADC; i++)
        { 
            clearbuf(p_i2s_adc[i]);
        }
#endif
    }
    else
    {
        clearbuf(p_bclk);
  

#if (I2S_CHANS_DAC != 0)
        /* Prefill the ports so data is input in advance */
        for(int i = 0; i < I2S_WIRES_DAC; i++)
        {
            p_i2s_dac[i] <: 0;
        }
#endif

        p_lrclk <: 0x7FFFFFFF; 
        p_bclk <: 0xAAAAAAAA;
        p_bclk <: 0xAAAAAAAA;
    }
#else          
    /* CODEC is master */
    /* Wait for LRCLK edge */
    p_lrclk when pinseq(0) :> void;
    p_lrclk when pinseq(1) :> void;
    p_lrclk when pinseq(0) :> void;
    p_lrclk when pinseq(1) :> void;
    p_lrclk when pinseq(0) :> void @ tmp;
    tmp += 33;
       
#if (I2S_CHANS_DAC != 0)
#pragma loop unroll 
    for(int i = 0; i < I2S_WIRES_DAC; i++)
    {
        p_i2s_dac[i] @ tmp <: 0;
    }
#endif

    p_i2s_adc[0] @ tmp - 1 :> void;  

#pragma loop unroll 
    for(int i = 0; i < I2S_WIRES_ADC; i++)
    { 
        clearbuf(p_i2s_adc[i]);
    }

    /* TODO In master mode, the i/o loop assumes L/RCLK = 32bit clocks.  We should check this every interation 
     * and resync if we got a bclk glitch */

#endif

    /* Main Audio I/O loop */    
    while (1)
    {
        outuint(c_out, 0);

        /* Check for sample freq change or new samples from mixer*/
        if(testct(c_out))
        {
            inct(c_out);
            return inuint(c_out);

        }
        else
        {
#ifndef MIXER // Interfaces straight to decouple()
            (void) inuint(c_out);

#if NUM_USB_CHAN_IN > 0
#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_IN; i++)
            {
                outuint(c_out, samplesIn[i]);
            }
#endif

#if NUM_USB_CHAN_OUT > 0
#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
            {
                samplesOut[i] = inuint(c_out);
            }
#endif
#else
#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
            {
                int tmp = inuint(c_out);
#if defined(OUT_VOLUME_IN_MIXER) && defined(OUT_VOLUME_AFTER_MIX)
                tmp<<=3;
#endif
                samplesOut[i] = tmp;
            }

#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_IN; i++)
            {
                outuint(c_out, samplesIn[i]);
            }
#endif
        }

#if defined(SPDIF_RX) || defined(ADAT_RX)
        inuint(c_dig_rx);
#endif
#ifdef SPDIF_RX
        asm("ldw %0, dp[g_digData]":"=r"(samplesIn[SPDIF_RX_INDEX + 0]));
        asm("ldw %0, dp[g_digData+4]":"=r"(samplesIn[SPDIF_RX_INDEX + 1]));

#endif
#ifdef ADAT_RX
        asm("ldw %0, dp[g_digData+8]":"=r"(samplesIn[ADAT_RX_INDEX + 0]));
        asm("ldw %0, dp[g_digData+12]":"=r"(samplesIn[ADAT_RX_INDEX+ 1]));
        asm("ldw %0, dp[g_digData+16]":"=r"(samplesIn[ADAT_RX_INDEX + 2]));
        asm("ldw %0, dp[g_digData+20]":"=r"(samplesIn[ADAT_RX_INDEX + 3]));
        asm("ldw %0, dp[g_digData+24]":"=r"(samplesIn[ADAT_RX_INDEX + 4]));
        asm("ldw %0, dp[g_digData+28]":"=r"(samplesIn[ADAT_RX_INDEX + 5]));
        asm("ldw %0, dp[g_digData+32]":"=r"(samplesIn[ADAT_RX_INDEX + 6]));
        asm("ldw %0, dp[g_digData+36]":"=r"(samplesIn[ADAT_RX_INDEX + 7]));
#endif

#if defined(SPDIF_RX) || defined(ADAT_RX)
        /* Request digital data (with prefill) */
        outuint(c_dig_rx, 0);
#endif



        tmp = 0;

#pragma xta endpoint "i2s_output_l"

#if (I2S_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT != 0)
#pragma loop unroll
        for(int i = 0; i < I2S_CHANS_DAC; i+=2)
        {           
            p_i2s_dac[tmp++] <: bitrev(samplesOut[i]);            /* Output LEFT sample to DAC */
        }
#endif
      
#ifndef CODEC_MASTER  
        /* Generate clocks LR Clock low - LEFT */ 
        switch (divide)
        {
           case 8:
       
               /* LR clock delayed by one clock, This is so MSB is output on the falling edge of BCLK 
                  * after the falling edge on which LRCLK was toggled. (see I2S spec) */
                p_lrclk <: 0x80000000;
      
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                break;
      
            case 4:
                p_lrclk <: 0x80000000;
      
                p_bclk <: 0xCCCCCCCC;
                p_bclk <: 0xCCCCCCCC;
                p_bclk <: 0xCCCCCCCC;
                p_bclk <: 0xCCCCCCCC;
                break;
      
            case 2: 
                p_lrclk <: 0x80000000;
          
                p_bclk <: 0xAAAAAAAA;
                p_bclk <: 0xAAAAAAAA;
                break;

            case 1:
                p_lrclk <: 0x80000000;
                break;
        }
#endif
  
 
#if (I2S_CHANS_ADC != 0)
        /* Input prevous R sample into R in buffer */
        index = 0;
#pragma loop unroll
        for(int i = 1; i < I2S_CHANS_ADC; i += 2)
        {
            p_i2s_adc[index++] :> sample;
#if NUM_USB_CHAN_IN > 0
            samplesIn[i] = bitrev(sample);

            /* Store the previous left in left */
            samplesIn[i-1] = samplesInPrev[i];
#endif
        }
#endif
        



#if defined(SPDIF) && (NUM_USB_CHAN_OUT > 0)	
        outuint(c_spd_out, samplesOut[SPDIF_TX_INDEX]);                 /* Forward sample to SPDIF txt thread */
        sample = samplesOut[SPDIF_TX_INDEX + 1];                 
        outuint(c_spd_out, sample);                 /* Forward sample to SPDIF txt thread */
#ifdef RAMP_CHECK
        sample >>= 8;
        if (started<10000) {
          if (sample == prev+1) 
            started++;
        }
        else
          if (sample != prev+1 && sample != 0) {
            printintln(prev);
            printintln(sample);
            printintln(prev-sample+1);
          }        
        prev = sample;
#endif

#endif

        tmp = 0;
#pragma xta endpoint "i2s_output_r"
#if (I2S_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT != 0)
#pragma loop unroll
        for(int i = 1; i < I2S_CHANS_DAC; i+=2)
        { 
            p_i2s_dac[tmp++] <: bitrev(samplesOut[i]);            /* Output RIGHT sample to DAC */
        }
#endif

#ifndef CODEC_MASTER 
        /* Clock out data (and LR clock) */
        switch (divide)
        {
            case 8: 
                p_lrclk <: 0x7FFFFFFF;

                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                break;
      
            case 4: 
                p_lrclk <: 0x7FFFFFFF;
          
                p_bclk <: 0xCCCCCCCC;
                p_bclk <: 0xCCCCCCCC;
                p_bclk <: 0xCCCCCCCC;
                p_bclk <: 0xCCCCCCCC;
                break;
      
            case 2: 
                p_lrclk <: 0x7FFFFFFF;
          
                p_bclk <: 0xAAAAAAAA;
                p_bclk <: 0xAAAAAAAA;
                break;

            case 1:
                p_lrclk <: 0x7FFFFFFF;
                break;
        }
#endif  
        
        


#if (I2S_CHANS_ADC != 0)
        /* Input previous L ADC sample */
        index = 0;
#pragma loop unroll
        for(int i = 1; i < I2S_CHANS_ADC; i += 2)
        {
            p_i2s_adc[index++] :> sample;

#if NUM_USB_CHAN_IN > 0
            samplesInPrev[i] = bitrev(sample);
#endif
        }

#ifdef SU1_ADC_ENABLE
        {
            unsigned x;

            x = inuint(c_adc);
            inct(c_adc);
            asm("stw %0, dp[g_adcVal]"::"r"(x));
        }
#endif
#endif
    }
    return 0;
}

/* This function is a dummy version of the deliver thread that does not 
   connect to the codec ports. It is used during DFU reset. */
static unsigned dummy_deliver(chanend c_out) {
    while (1)
    {
        outuint(c_out, 0);

        /* Check for sample freq change or new samples from mixer*/
        if(testct(c_out))
        {
            inct(c_out);
            return inuint(c_out);

        }
        else
        {
#ifndef MIXER // Interfaces straight to decouple()
            (void) inuint(c_out);
#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_IN; i++)
            {
                outuint(c_out, 0);
            }

#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
            {
                (void) inuint(c_out);
            }
#else
#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
            {
                (void) inuint(c_out);
            }

#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_IN; i++)
            {
                outuint(c_out, 0);
            }
#endif
        }
    }
    return 0;
}
#define SAMPLE_RATE      200000
#define NUMBER_CHANNELS  1
#define NUMBER_SAMPLES  100
#define NUMBER_WORDS ((NUMBER_SAMPLES * NUMBER_CHANNELS+1)/2)
#define SAMPLES_PER_PRINT 1


void audio(chanend c_mix_out, chanend ?c_dig_rx, chanend ?c_config, chanend ?c) 
{
#ifdef SPDIF
    chan c_spdif_out;
#endif
    unsigned curSamFreq = DEFAULT_FREQ;
    unsigned mClk;
    unsigned divide;
    unsigned firstRun = 1;

#ifdef SU1_ADC_ENABLE
    /* Setup galaxian ADC */
    unsigned data[1],  channel;
    int r;
    unsigned int vals[NUMBER_WORDS];
    int cnt = 0;
    int div;
    unsigned val = 0;
    int val2 = 0;
    int adcOk = 0;

    /* Enable adc on channel */
    enable_xs1_su_adc_input(0, c);

    /* General ADC control (enabled, 1 samples per packet, 32 bits per sample) */
    data[0] = 0x10201;
    data[0] = 0x30101;
    r = write_periph_32(xs1_su, 2, 0x20, 1, data);

    /* ADC needs a few clocks before it starts pumping out samples */
    for(int i = 0; i< 10; i++)
    {
        p_lrclk <: val;
        val = ~val;
        {
            timer t;
            unsigned time;
            t :> time;
            t when timerafter(time+1000):> void;
        }
    }
#endif

#ifdef SPDIF
    SpdifTransmitPortConfig(p_spdif_tx, clk_mst_spd, p_mclk);
#endif

  	/* Initialise master clock generation */
    ClockingInit(c_config);

    /* Perform required CODEC/ADC/DAC initialisation */
    CodecInit(c_config);

    while(1)
    {
      
        /* Calculate what master clock we should be using */
        if ((curSamFreq % 22050) == 0)
        {
            mClk = MCLK_441;
        }
        else if ((curSamFreq % 24000) == 0)
        {
            mClk = MCLK_48;
        }

        /* Calculate divide required for bit clock e.g. 11.289600 / (176400 * 64)  = 1 */
        divide = mClk / ( curSamFreq * 64 );

        /* Configure clocking for required master clock */
        ClockingConfig(mClk, c_config); 

        if(!firstRun)
        {
            /* TODO wait for good mclk instead of delay */ 
            /* No delay for DFU modes */
            if ((curSamFreq != AUDIO_REBOOT_FROM_DFU) && (curSamFreq != AUDIO_STOP_FOR_DFU)) 
            {
                timer t;
                unsigned time;
                t :> time;
                t when timerafter(time+AUDIO_PLL_LOCK_DELAY) :> void;

                /* Handshake back */
                outct(c_mix_out, XS1_CT_END);
            }
        } 
        firstRun = 0;
      
        /* Configure CODEC/DAC/ADC for SampleFreq/MClk */
        CodecConfig(curSamFreq, mClk, c_config);

        /* Configure audio ports */
        ConfigAudioPorts(divide);    

        par
        {
            
#ifdef SPDIF   
            {  
                set_thread_fast_mode_on();
                SpdifTransmit(p_spdif_tx, c_spdif_out);
            }
#endif  
  
            {     
#ifdef SPDIF
                /* Communicate master clock and sample freq to S/PDIF thread */
                outuint(c_spdif_out, curSamFreq);
                outuint(c_spdif_out, mClk);
#endif 

                curSamFreq = deliver(c_mix_out,
#ifdef SPDIF
                   c_spdif_out,
#else
                   null,
#endif 
                   divide, c_dig_rx, c);

                // Currently no more audio will happen after this point
                if (curSamFreq == AUDIO_STOP_FOR_DFU) 
				{
                  	outct(c_mix_out, XS1_CT_END);

                  	while (1) 
					{

                    	curSamFreq = dummy_deliver(c_mix_out);

                    	if (curSamFreq == AUDIO_START_FROM_DFU) 
						{
                      		outct(c_mix_out, XS1_CT_END);
                      		break;
                    	}
                  	}
                }

#ifdef SPDIF 
                /* Notify S/PDIF thread of impending new freq... */
                outct(c_spdif_out, XS1_CT_END);   
#endif
            }
        }
	}
}

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
#include <xs1_su.h>

#include "devicedefines.h"
#include "audioports.h"
#include "audiohw.h"
#ifdef SPDIF
#include "SpdifTransmit.h"
#endif
#include "commands.h"
#include "xc_ptr.h"

unsigned testsamples[100];
int p = 0;
unsigned lastSample = 0;
#if (DSD_CHANS_DAC != 0) 
extern unsigned p_dsd_dac[DSD_CHANS_DAC];
extern port p_dsd_clk;
#endif

unsigned g_adcVal = 0;

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

unsigned dsdMode = DSD_MODE_OFF;

/* Master clock input */
extern port p_mclk_in;

#ifdef SPDIF 
extern buffered out port:32 p_spdif_tx;
#endif

extern clock    clk_audio_mclk;  
extern clock    clk_audio_bclk;  
extern clock    clk_mst_spd;  

extern void device_reboot(void);

#define MAX_DIVIDE_48 (MCLK_48/MIN_FREQ_48/64)
#define MAX_DIVIDE_44 (MCLK_44/MIN_FREQ_44/64)
#if (MAX_DIVIDE_44 > MAX_DIVIDE_48)
#define MAX_DIVIDE (MAX_DIVIDE_44)
#else
#define MAX_DIVIDE (MAX_DIVIDE_48)
#endif

static inline void doI2SClocks(unsigned divide)
{ 
    switch (divide)
    {
#if (MAX_DIVIDE > 16)
#error MCLK/BCLK Ratio not supported!! 
#endif
#if (MAX_DIVIDE > 8)
             case 16:
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00;   
                p_bclk <: 0xff00ff00; 
                break;
#endif
#if (MAX_DIVIDE > 4)
           case 8:
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                p_bclk <: 0xF0F0F0F0;   
                break;
#endif
#if (MAX_DIVIDE > 2)
            case 4:
                p_bclk <: 0xCCCCCCCC;
                p_bclk <: 0xCCCCCCCC;
                p_bclk <: 0xCCCCCCCC;
                p_bclk <: 0xCCCCCCCC;
                break;
#endif
#if (MAX_DIVIDE > 1)
            case 2: 
                p_bclk <: 0xAAAAAAAA;
                p_bclk <: 0xAAAAAAAA;
                break;
#endif
#if (MAX_DIVIDE > 0)
            case 1:
                break;
#endif     
   }
}



/* I2S delivery thread */
#pragma unsafe arrays
{unsigned, unsigned} deliver(chanend c_out, chanend ?c_spd_out, unsigned divide, chanend ?c_dig_rx, chanend ?c_adc)
{
	unsigned sample;
    unsigned underflow = 0;
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

#if (DSD_CHANS_DAC != 0)
    unsigned dsdMarker = DSD_MARKER_2;    /* This alternates between DSD_MARKER_1 and DSD_MARKER_2 */
    int dsdCount = 0;
    int everyOther = 1;
    unsigned dsdSample_l = 0x96960000;
    unsigned dsdSample_r = 0x96960000;
#endif
    int counter = 0;
    unsigned underflowWord = 0;

#if NUM_USB_CHAN_IN > 0
    for (int i=0;i<NUM_USB_CHAN_IN;i++)
    {
        samplesIn[i] = 0;
        samplesInPrev[i] = 0;
    }
#endif

#if(DSD_CHANS_DAC != 0) 
    if(dsdMode == DSD_MODE_DOP)
        underflowWord = 0xFA969600;
    else if(dsdMode == DSD_MODE_NATIVE)
        underflowWord = 0x96969696;
#endif


    outuint(c_out, 0);

    /* Check for sample freq change or new samples from mixer*/
    if(testct(c_out))
    {
        unsigned command = inct(c_out);
        
            // Set clocks low
            p_lrclk <: 0;
            p_bclk <: 0;
#if(DSD_CHANS_DAC != 0) 
            /* DSD Clock might not be shared with lrclk or bclk... */
            p_dsd_clk <: 0;
#endif
#if (DSD_CHANS_DAC > 0)
        if(dsdMode == DSD_MODE_DOP)
            dsdMode = DSD_MODE_OFF; 
#endif
        return {command, inuint(c_out)};
    }
    else
    {
#ifndef MIXER // Interfaces straight to decouple()
        underflow = inuint(c_out);

#if NUM_USB_CHAN_IN > 0
#pragma loop unroll
        for(int i = 0; i < NUM_USB_CHAN_IN; i++)
        {
            outuint(c_out, samplesIn[i]);
        }
#endif

#if NUM_USB_CHAN_OUT > 0
        if(underflow)
        {
#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_OUT; i++)        
            {
                samplesOut[i] = underflowWord;
            }
        }
        else
        {
#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_OUT; i++)        
            {
                samplesOut[i] = inuint(c_out);
            }
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

#if (DSD_CHANS_DAC > 0)
    if(dsdMode == DSD_MODE_OFF)
    {
#endif
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
        tmp += 100;

        /* Since BCLK is free-running, setup outputs/inputs at a know point in the future */ 
#if (I2S_CHANS_DAC != 0)
#pragma loop unroll
        for(int i = 0; i < I2S_WIRES_DAC; i++)
        {
            p_i2s_dac[i] @ tmp <: 0;
        }
#endif

        p_lrclk @ tmp <: 0x7FFFFFFF;  


#if (I2S_CHANS_ADC != 0)
        for(int i = 0; i < I2S_WIRES_DAC; i++)
        {
            //p_i2s_adc[0]  @ (tmp - 1) :> void;  
            asm("setpt res[%0], %1"::"r"(p_i2s_adc[i]),"r"(tmp-1));
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
        doI2SClocks(divide);
        
    }
#if (DSD_CHANS_DAC > 0)
    } /* if (!dsdMode) */
#endif
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

        
        /* Check for sample freq change (or other command) or new samples from mixer*/
        if(testct(c_out))
        {
            unsigned command;

            // Set clocks low
            p_lrclk <: 0;
            p_bclk <: 0;
#if(DSD_CHANS_DAC != 0) 
            /* DSD Clock might not be shared with lrclk or bclk... */
            p_dsd_clk <: 0;
#endif
            command = inct(c_out);
            
#if (DSD_CHANS_DAC > 0)
            if(dsdMode == DSD_MODE_DOP)
                dsdMode = DSD_MODE_OFF;
#endif
            return {command, inuint(c_out)};

        }
        else
        {
#ifndef MIXER // Interfaces straight to decouple()
            underflow = inuint(c_out);
            counter++;
#if NUM_USB_CHAN_IN > 0
#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_IN; i++)
            {
                outuint(c_out, samplesIn[i]);
            }
#endif

#if NUM_USB_CHAN_OUT > 0
            if(underflow)
            {
#pragma loop unroll
                for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
                {
                    samplesOut[i] = underflowWord;
                }
            }
            else
            {
#pragma loop unroll
                for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
                {
                    samplesOut[i] = inuint(c_out);
                }
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
#if (DSD_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT > 0)
    if(dsdMode == DSD_MODE_NATIVE)
    {
        /* 8 bits per chan, 1st 1-bit sample in MSB */
        dsdSample_l =  samplesOut[0];  
        dsdSample_r =  samplesOut[1];
        dsdSample_r = bitrev(byterev(dsdSample_r)); 
        dsdSample_l = bitrev(byterev(dsdSample_l)); 
 
            switch (divide)
            {
                case 8:
                    /* Output DSD data to ports then 32 clocks */
                    asm volatile("out res[%0], %1"::"r"(p_dsd_dac[0]),"r"(dsdSample_l));
                    asm volatile("out res[%0], %1"::"r"(p_dsd_dac[1]),"r"(dsdSample_r));
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    break;
      
                case 4:
                    asm volatile("out res[%0], %1"::"r"(p_dsd_dac[0]),"r"(dsdSample_l));
                    asm volatile("out res[%0], %1"::"r"(p_dsd_dac[1]),"r"(dsdSample_r));
                    p_dsd_clk <: 0xCCCCCCCC;
                    p_dsd_clk <: 0xCCCCCCCC;
                    p_dsd_clk <: 0xCCCCCCCC;
                    p_dsd_clk <: 0xCCCCCCCC;
                    break;
      
                case 2: 
                    asm volatile("out res[%0], %1"::"r"(p_dsd_dac[0]),"r"(dsdSample_l));
                    asm volatile("out res[%0], %1"::"r"(p_dsd_dac[1]),"r"(dsdSample_r));
                    p_dsd_clk <: 0xAAAAAAAA;
                    p_dsd_clk <: 0xAAAAAAAA;
                    break;
                
                default:
                    /* Do some clocks anyway - this will stop us interrupting decouple too much */
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0; 
                    break;
            } 

    }
    else if(dsdMode == DSD_MODE_DOP)
    {
        if(!everyOther)
        {
            dsdSample_l = ((samplesOut[0] & 0xffff00) << 8);
            dsdSample_r = ((samplesOut[1] & 0xffff00) << 8);
 
            everyOther = 1;
          
            switch (divide)
            {
                case 8:
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    break;
      
                case 4:
                    p_dsd_clk <: 0xCCCCCCCC;
                    p_dsd_clk <: 0xCCCCCCCC;
                    break;
      
                case 2: 
                    p_dsd_clk <: 0xAAAAAAAA;
                    break;
                case 1:
                    break;
            } 
        }
        else if(everyOther)
        {
            everyOther = 0;
            dsdSample_l =  dsdSample_l | ((samplesOut[0] & 0xffff00) >> 8);   
            dsdSample_r =  dsdSample_r | ((samplesOut[1] & 0xffff00) >> 8);   

            // Output 16 clocks DSD to all
            //p_dsd_dac[0] <: bitrev(dsdSample_l);
            //p_dsd_dac[1] <: bitrev(dsdSample_r);
            asm volatile("out res[%0], %1"::"r"(p_dsd_dac[0]),"r"(bitrev(dsdSample_l)));
            asm volatile("out res[%0], %1"::"r"(p_dsd_dac[1]),"r"(bitrev(dsdSample_r)));
            switch (divide)
            {
                case 8:
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    p_dsd_clk <: 0xF0F0F0F0;   
                    break;
      
                case 4:
                    p_dsd_clk <: 0xCCCCCCCC;
                    p_dsd_clk <: 0xCCCCCCCC;
                    break;
      
                case 2: 
                    p_dsd_clk <: 0xAAAAAAAA;
                    break;
                case 1:
                    break;
            } 

        }
    }
    else
#endif
    {

#pragma xta endpoint "i2s_output_l"

#if (I2S_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT != 0)
#pragma loop unroll
        for(int i = 0; i < I2S_CHANS_DAC; i+=2)
        {           
            p_i2s_dac[tmp++] <: bitrev(samplesOut[i]);            /* Output LEFT sample to DAC */
        }
#endif
      
#ifndef CODEC_MASTER  
        /* LR clock delayed by one clock, This is so MSB is output on the falling edge of BCLK 
         * after the falling edge on which LRCLK was toggled. (see I2S spec) */
        /* Generate clocks LR Clock low - LEFT */ 
        p_lrclk <: 0x80000000;
        doI2SClocks(divide);
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
        outuint(c_spd_out, samplesOut[SPDIF_TX_INDEX]);  /* Forward sample to S/PDIF Tx thread */
        sample = samplesOut[SPDIF_TX_INDEX + 1];                 
        outuint(c_spd_out, sample);                      /* Forward sample to S/PDIF Tx thread */
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
        p_lrclk <: 0x7FFFFFFF;
        doI2SClocks(divide);
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

        }  // !dsdMode       
#if (DSD_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT > 0)
        /* Check for DSD */
        /* Currently we only check on channel 0 - we get all 0's on channels without data */
        if(dsdMode == DSD_MODE_OFF)
        {
            if((DSD_MASK(samplesOut[0]) == dsdMarker) && (DSD_MASK(samplesOut[1]) == dsdMarker))
            {
                dsdCount++;
                dsdMarker ^= DSD_MARKER_XOR;
                if(dsdCount == DSD_EN_THRESH)
                {
                    dsdMode = DSD_MODE_DOP;
                    dsdCount = 0;
                    dsdMarker = DSD_MARKER_2;
                    
                    // Set clocks low
                    p_lrclk <: 0;
                    p_bclk <: 0;
                    p_dsd_clk <: 0;
                    return {0,0};
                }
            }
            else
            {
                dsdCount = 0;
                dsdMarker = DSD_MARKER_2;
            }
        }
        else if(dsdMode == DSD_MODE_DOP) // DSD DoP Mode
        {
            /* If we are running in DOP mode, check if we need to come out */
            if((DSD_MASK(samplesOut[0]) != DSD_MARKER_1) && (DSD_MASK(samplesOut[1]) != DSD_MARKER_1))
            {
                if((DSD_MASK(samplesOut[0]) != DSD_MARKER_2) && (DSD_MASK(samplesOut[1]) != DSD_MARKER_2))
                {
                    dsdMode = DSD_MODE_OFF;
                    // Set clocks low
                    p_lrclk <: 0;
                    p_bclk <: 0;
                    p_dsd_clk <: 0;
                    return {0,0};
                }
            }
        }
    
#endif

    }
    return {0,0};
}

/* This function is a dummy version of the deliver thread that does not 
   connect to the codec ports. It is used during DFU reset. */
{unsigned,unsigned} dummy_deliver(chanend c_out) 
{
    while (1)
    {
        outuint(c_out, 0);

        /* Check for sample freq change or new samples from mixer*/
        if(testct(c_out))
        {
            unsigned command = inct(c_out);
             return {command, inuint(c_out)};

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
    return {0,0};
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
    unsigned retVal1, retVal2;
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

    /* Clock master clock-block from master-clock port */
    configure_clock_src(clk_audio_mclk, p_mclk_in);

    start_clock(clk_audio_mclk);

#ifdef SPDIF
    SpdifTransmitPortConfig(p_spdif_tx, clk_mst_spd, p_mclk_in);
#endif

    /* Perform required CODEC/ADC/DAC initialisation */
    AudioHwInit(c_config);

    while(1)
    {
        /* Calculate what master clock we should be using */
        if ((MCLK_441 % curSamFreq) == 0)
        {
            mClk = MCLK_441;
        }
        else if ((MCLK_48 % curSamFreq) == 0)
        {
            mClk = MCLK_48;
        }

        /* Calculate master clock to bit clock (or DSD clock) divide for current sample freq
         * e.g. 11.289600 / (176400 * 64)  = 1 */
        {
            /* I2S has 32 bits per sample. *2 as 2 channels */
            unsigned numBits = 64;
 
#if (DSD_CHANS_DAC > 0)
            if(dsdMode == DSD_MODE_DOP)
            {
                /* DoP we receive in 16bit chunks */
                numBits = 16;
            }
            else if(dsdMode == DSD_MODE_NATIVE)
            {
                /* DSD native we receive in 32bit chunks */
                numBits = 32;
            }
#endif  
            divide = mClk / ( curSamFreq * numBits );
 
            //if(divide > 8)
                //asm("ecallf %0"::"r"(0));
            
       } 
        
#if (DSD_CHANS_DAC != 0)
        /* Configure audio ports */
        ConfigAudioPortsWrapper(
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
            divide, dsdMode);
#else
 /* Configure audio ports */
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

#endif
 
        {
            unsigned curFreq = curSamFreq;         
#if (DSD_CHANS_DAC > 0)  
            /* Make AudioHwConfig() implementation a little more user friendly in DSD mode...*/ 
            if(dsdMode == DSD_MODE_NATIVE)
            {
                curFreq *= 32;                      
            }
            else if(dsdMode == DSD_MODE_DOP)
            {
                curFreq *= 16;                      
            }
#endif
            /* Configure Clocking/CODEC/DAC/ADC for SampleFreq/MClk */
            AudioHwConfig(curFreq, mClk, c_config, dsdMode);
        }

        if(!firstRun)
        {
            /* TODO wait for good mclk instead of delay */ 
            /* No delay for DFU modes */
            if ((curSamFreq != AUDIO_REBOOT_FROM_DFU) && (curSamFreq != AUDIO_STOP_FOR_DFU) && retVal1) 
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

                {retVal1, retVal2} = deliver(c_mix_out,
#ifdef SPDIF
                   c_spdif_out,
#else
                   null,
#endif 
                   divide, c_dig_rx, c);

#if (DSD_CHANS_DAC != 0)
                if(retVal1 == SET_SAMPLE_FREQ)
                {
                    curSamFreq = retVal2;
                }
                else if(retVal1 == SET_DSD_MODE)
                {
                    /* Off = 0
                     * DOP = 1
                     * Native = 2
                     */
                    dsdMode = retVal2;
                }
#else
                curSamFreq = retVal2;
#endif

                // Currently no more audio will happen after this point
                if (curSamFreq == AUDIO_STOP_FOR_DFU) 
				{
                  	outct(c_mix_out, XS1_CT_END);

                  	while (1) 
					{

                    	{retVal1, curSamFreq} = dummy_deliver(c_mix_out);

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

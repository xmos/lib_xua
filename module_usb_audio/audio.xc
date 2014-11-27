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


static unsigned samplesOut[NUM_USB_CHAN_OUT];

/* Two buffers for ADC data to allow for DAC and ADC ports being offset */
static unsigned samplesIn_0[NUM_USB_CHAN_IN];
static unsigned samplesIn_1[I2S_CHANS_ADC];

#ifdef I2S_MODE_TDM
#define I2S_CHANS_PER_FRAME 8
#else
#define I2S_CHANS_PER_FRAME 2
#endif

#if (DSD_CHANS_DAC != 0)
extern buffered out port:32 p_dsd_dac[DSD_CHANS_DAC];
extern buffered out port:32 p_dsd_clk;
#endif

unsigned g_adcVal = 0;


#ifdef XTA_TIMING_AUDIO
#pragma xta command "add exclusion received_command"
#pragma xta command "analyse path i2s_output_l i2s_output_r"
#pragma xta command "set required - 2000 ns"

#pragma xta command "add exclusion received_command"
#pragma xta command "add exclusion received_underflow"
#pragma xta command "add exclusion divide_1"
#pragma xta command "add exclusion deliver_return"
#pragma xta command "analyse path i2s_output_r i2s_output_l"
#pragma xta command "set required - 2000 ns"
#endif

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

#ifndef CODEC_MASTER
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
#endif

#pragma unsafe arrays
static inline unsigned DoSampleTransfer(chanend c_out, int readBuffNo, unsigned underflowWord)
{
    unsigned command;
    unsigned underflow;

     outuint(c_out, 0);

        /* Check for sample freq change (or other command) or new samples from mixer*/
        if(testct(c_out))
        {
            unsigned command = inct(c_out);
#ifndef CODEC_MASTER
            // Set clocks low
            p_lrclk <: 0;
            p_bclk <: 0;
#if(DSD_CHANS_DAC != 0)
            /* DSD Clock might not be shared with lrclk or bclk... */
            p_dsd_clk <: 0;
#endif
#endif
#if (DSD_CHANS_DAC > 0)
            if(dsdMode == DSD_MODE_DOP)
                dsdMode = DSD_MODE_OFF;
#endif
#pragma xta endpoint "received_command"
            return command;

        }
        else
        {
            underflow = inuint(c_out);
#ifndef MIXER // Interfaces straight to decouple()
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
#else /* ifndef MIXER */
#if NUM_USB_CHAN_OUT > 0
            if(underflow)
            {
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
                    int tmp = inuint(c_out);
                    samplesOut[i] = tmp;
                }
            }
#endif
#if NUM_USB_CHAN_IN > 0
#pragma loop unroll
            for(int i = 0; i < I2S_CHANS_ADC; i++)
            {
                if(readBuffNo)
                    outuint(c_out, samplesIn_1[i]);
                else
                    outuint(c_out, samplesIn_0[i]);
            }
            /* Send over the digi channels - no odd buffering required */
#pragma loop unroll
            for(int i = I2S_CHANS_ADC; i < NUM_USB_CHAN_IN; i++)
            {
                outuint(c_out, samplesIn_0[i]);
            }
#endif
#endif
        }

        return 0;

}

static inline void InitPorts(unsigned divide)
{
    unsigned tmp;
#ifndef CODEC_MASTER
#if (DSD_CHANS_DAC > 0)
    if(dsdMode == DSD_MODE_OFF)
    {
#endif
        /* b_clk must start high */
        p_bclk <: 0x80000000;
        sync(p_bclk);
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
#pragma xta endpoint "divide_1"
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

            p_lrclk @ tmp <: 0x7FFFFFFF;


#if (I2S_CHANS_ADC != 0)
            for(int i = 0; i < I2S_WIRES_ADC; i++)
            {
                asm("setpt res[%0], %1"::"r"(p_i2s_adc[i]),"r"(tmp-1));
            }
#endif
        }
        else /* Divide != 1  */
        {
#if (I2S_CHANS_DAC != 0)
            /* Pre-fill the DAC ports */
            for(int i = 0; i < I2S_WIRES_DAC; i++)
            {
                p_i2s_dac[i] <: 0;
            }
#endif
            /* Pre-fill the LR clock output port */
            p_lrclk <: 0x0;
            
            doI2SClocks(divide);

        }
#if (DSD_CHANS_DAC > 0)
    } /* if (!dsdMode) */
    else
    {
        /* p_dsd_clk must start high */
        p_dsd_clk <: 0x80000000;
    }
#endif
#else /* ifndef CODEC_MASTER */

    /* Wait for LRCLK edge */
    p_lrclk when pinseq(0) :> void;
    p_lrclk when pinseq(1) :> void;
    p_lrclk when pinseq(0) :> void;
    p_lrclk when pinseq(1) :> void;
    p_lrclk when pinseq(0) :> void @ tmp;
    tmp+=97;
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
        asm("setpt res[%0], %1"::"r"(p_i2s_adc[i]),"r"(tmp+31));
    }
#endif
#endif
}



/* I2S delivery thread */
#pragma unsafe arrays
unsigned static deliver(chanend c_out, chanend ?c_spd_out, unsigned divide, unsigned curSamFreq,
#if(defined(SPDIF_RX) || defined(ADAT_RX))
chanend c_dig_rx,
#endif
chanend ?c_adc)
{
#if (I2S_CHANS_ADC != 0) || defined(SPDIF)
	unsigned sample;
#endif
    unsigned underflow = 0;
#if NUM_USB_CHAN_OUT > 0
#endif
//#if NUM_USB_CHAN_IN > 0
    /* Since DAC and ADC buffered ports off by one sample we buffer previous ADC frame */
    unsigned readBuffNo = 0;
//#endif
    unsigned tmp;
    unsigned index;

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
    unsigned underflowWord = 0;

    unsigned frameCount = 0;

#if(DSD_CHANS_DAC != 0)
    if(dsdMode == DSD_MODE_DOP)
    {
        underflowWord = 0xFA969600;
    }
    else if(dsdMode == DSD_MODE_NATIVE)
    {
        underflowWord = 0x96969696;
    }
#endif

#if 1
    unsigned command = DoSampleTransfer(c_out, readBuffNo, underflowWord);       

    if(command)
    {
        return command;
    }
#endif

    InitPorts(divide);

    /* TODO In master mode, the i/o loop assumes L/RCLK = 32bit clocks.  We should check this every interation
     * and resync if we got a bclk glitch */

    /* Main Audio I/O loop */
    while (1)
    {

#if (DSD_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT > 0)
        if(dsdMode == DSD_MODE_NATIVE)
        {
            /* 8 bits per chan, 1st 1-bit sample in MSB */
            dsdSample_l =  samplesOut[0];
            dsdSample_r =  samplesOut[1];
            dsdSample_r = bitrev(byterev(dsdSample_r));
            dsdSample_l = bitrev(byterev(dsdSample_l));

            /* Output DSD data to ports then 32 clocks */
            switch (divide)
            {
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
                }
            }
            else // everyOther
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
                }

            }
        }
        else
#endif
        {
#ifndef CODEC_MASTER
            /* LR clock delayed by one clock, This is so MSB is output on the falling edge of BCLK
             * after the falling edge on which LRCLK was toggled. (see I2S spec) */
            /* Generate clocks LR Clock low - LEFT */
#ifdef I2S_MODE_TDM
            p_lrclk <: 0x00000000;
#else
            p_lrclk <: 0x80000000;

#endif
#endif

#pragma xta endpoint "i2s_output_l"

#if (I2S_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT != 0)
            index = 0;
#pragma loop unroll
#ifdef I2S_MODE_TDM
            for(int i = 0; i < I2S_CHANS_DAC; i+=8)
            {
                p_i2s_dac[index++] <: bitrev(samplesOut[(frameCount)+i]);
            }
#else
            for(int i = 0; i < I2S_CHANS_DAC; i+=2)
            {
                p_i2s_dac[index++] <: bitrev(samplesOut[i]);            /* Output Left sample to DAC */
            }
#endif
#endif
            
            /* Clock out the LR Clock, the DAC data and Clock in the next sample into ADC */
            doI2SClocks(divide);

#if (I2S_CHANS_ADC != 0)
            /* Input previous L sample into L in buffer */
            index = 0;
            /* First input (i.e. frameCoint == 0) we read last ADC channel of previous frame.. */
            unsigned buffIndex = frameCount ? !readBuffNo : readBuffNo;

#pragma loop unroll           
            /* First time around we get channel 7 of TDM8 */
            for(int i = 0; i < I2S_CHANS_ADC; i+=I2S_CHANS_PER_FRAME)
            {
                asm volatile("in %0, res[%1]" : "=r"(sample)  : "r"(p_i2s_adc[index++]));

                /* Note the use of readBuffNo changes based on frameCount */
                if(buffIndex)
                    samplesIn_1[((frameCount-1)&(I2S_CHANS_PER_FRAME-1))+i] = bitrev(sample); // channels 1, 3, 5.. on each line.
                else
                    samplesIn_0[((frameCount-1)&(I2S_CHANS_PER_FRAME-1))+i] = bitrev(sample); // channels 1, 3, 5.. on each line.
            }
#endif

        
#if defined(SPDIF_RX) || defined(ADAT_RX)
            /* Sync with clockgen */
            inuint(c_dig_rx);

            /* Note, digi-data we just store in samplesIn_0 - we only double buffer the I2S input data */
#endif
#ifdef SPDIF_RX
            asm("ldw %0, dp[g_digData]":"=r"(samplesIn_0[SPDIF_RX_INDEX + 0]));
            asm("ldw %0, dp[g_digData+4]":"=r"(samplesIn_0[SPDIF_RX_INDEX + 1]));

#endif
#ifdef ADAT_RX
            asm("ldw %0, dp[g_digData+8]":"=r"(samplesIn_0[ADAT_RX_INDEX]));
            asm("ldw %0, dp[g_digData+12]":"=r"(samplesIn_0[ADAT_RX_INDEX + 1]));
            asm("ldw %0, dp[g_digData+16]":"=r"(samplesIn_0[ADAT_RX_INDEX + 2]));
            asm("ldw %0, dp[g_digData+20]":"=r"(samplesIn_0[ADAT_RX_INDEX + 3]));
            asm("ldw %0, dp[g_digData+24]":"=r"(samplesIn_0[ADAT_RX_INDEX + 4]));
            asm("ldw %0, dp[g_digData+28]":"=r"(samplesIn_0[ADAT_RX_INDEX + 5]));
            asm("ldw %0, dp[g_digData+32]":"=r"(samplesIn_0[ADAT_RX_INDEX + 6]));
            asm("ldw %0, dp[g_digData+36]":"=r"(samplesIn_0[ADAT_RX_INDEX + 7]));
#endif

#if defined(SPDIF_RX) || defined(ADAT_RX)
        /* Request digital data (with prefill) */
        outuint(c_dig_rx, 0);
#endif
#if defined(SPDIF) && (NUM_USB_CHAN_OUT > 0)
            outuint(c_spd_out, samplesOut[SPDIF_TX_INDEX]);  /* Forward sample to S/PDIF Tx thread */
            sample = samplesOut[SPDIF_TX_INDEX + 1];
            outuint(c_spd_out, sample);                      /* Forward sample to S/PDIF Tx thread */
#endif
            
           

#ifndef CODEC_MASTER
#ifdef I2S_MODE_TDM
            if(frameCount == (I2S_CHANS_PER_FRAME-2))
                p_lrclk <: 0x80000000;
            else
               p_lrclk <: 0x00000000;
#else
            p_lrclk <: 0x7FFFFFFF;
#endif
#endif

            index = 0;
#pragma xta endpoint "i2s_output_r"
#if (I2S_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT != 0)
#pragma loop unroll
#ifdef I2S_MODE_TDM
            for(int i = 0; i < I2S_CHANS_DAC; i+=I2S_CHANS_PER_FRAME)
            {
                p_i2s_dac[index++] <: bitrev(samplesOut[frameCount+1+i]);
            }
#else
            for(int i = 1; i < I2S_CHANS_DAC; i+=2)
            {
                p_i2s_dac[index++] <: bitrev(samplesOut[i]);            /* Output Right sample to DAC */
            }
#endif
#endif

            doI2SClocks(divide);

#if (I2S_CHANS_ADC != 0)
            index = 0;
            /* Channels 0, 2, 4.. on each line */
#pragma loop unroll
            for(int i = 0; i < I2S_CHANS_ADC; i += I2S_CHANS_PER_FRAME)
            {
                /* Manual IN instruction since compiler generates an extra setc per IN (bug #15256) */
                asm volatile("in %0, res[%1]" : "=r"(sample)  : "r"(p_i2s_adc[index++]));
                if(readBuffNo)
                    samplesIn_0[frameCount+i] = bitrev(sample);             
                else
                    samplesIn_1[frameCount+i] = bitrev(sample);             
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
        /* Check for DSD - note we only move into DoP mode if valid DoP Freq */
        /* Currently we only check on channel 0 - we get all 0's on channels without data */
        if((dsdMode == DSD_MODE_OFF) && (curSamFreq > 96000))
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
                    return 0;
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
                    return 0;
                }
            }
        }
#endif
        
#ifdef I2S_MODE_TDM
        /* Increase frameCount by 2 since we have output two channels (per data line) */
        frameCount+=2;
        if(frameCount == I2S_CHANS_PER_FRAME)
#endif
        {
            /* Do samples transfer */
            /* The below looks a bit odd but forces the compiler to inline twice */
            unsigned command;
            if(readBuffNo)
                command = DoSampleTransfer(c_out, 1, underflowWord);       
            else
                command = DoSampleTransfer(c_out, 0, underflowWord);       


            if(command)
            {
                return command;
            }
            
            /* Reset frame counter and flip the ADC buffer */
            frameCount = 0;
            readBuffNo = !readBuffNo;
        }
    }

#pragma xta endpoint "deliver_return"
    return 0;
}

/* This function is a dummy version of the deliver thread that does not
   connect to the codec ports. It is used during DFU reset. */
unsigned static dummy_deliver(chanend c_out)
{
    while (1)
    {
        outuint(c_out, 0);

        /* Check for sample freq change or new samples from mixer*/
        if(testct(c_out))
        {
            unsigned command = inct(c_out);
            return command;
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

void audio(chanend c_mix_out,
#if (defined(ADAT_RX) || defined(SPDIF_RX))
chanend c_dig_rx,
#endif
chanend ?c_config, chanend ?c)
{
#ifdef SPDIF
    chan c_spdif_out;
#endif
    unsigned curSamFreq = DEFAULT_FREQ;
    unsigned curSamRes_DAC = STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS; /* Default to something reasonable */
    unsigned curSamRes_ADC = STREAM_FORMAT_INPUT_1_RESOLUTION_BITS; /* Default to something reasonable - note, currently this never changes*/
    unsigned command;
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

#if (DSD_CHANS_DAC > 0)
    /* Make sure the DSD ports are on and buffered - just in case they are not shared with I2S */
    EnableBufferedPort(p_dsd_clk, 32);
    for(int i = 0; i< DSD_CHANS_DAC; i++)
    {
        EnableBufferedPort(p_dsd_dac[i], 32);
    }
#endif

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
#if I2S_MODE_TDM
            /* I2S has 32 bits per sample. *8 as 8 channels */
            unsigned numBits = 256;
#else
            /* I2S has 32 bits per sample. *2 as 2 channels */
            unsigned numBits = 64;
#endif

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
            divide = mClk / ( curSamFreq * numBits);
       }


#if (DSD_CHANS_DAC > 0)
        if(dsdMode)
        {
        /* Configure audio ports */
        ConfigAudioPortsWrapper(
#if (I2S_CHANS_DAC != 0)
                p_dsd_dac,
                DSD_CHANS_DAC,
#endif
#if (I2S_CHANS_ADC != 0)
                p_i2s_adc,
                I2S_WIRES_ADC,
#endif
#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
                null,
                p_dsd_clk,
#endif
                divide, dsdMode);
        }
        else
#endif
        {

            ConfigAudioPortsWrapper(
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
            divide, dsdMode);
}


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
            AudioHwConfig(curFreq, mClk, c_config, dsdMode, curSamRes_DAC, curSamRes_ADC);
        }

        if(!firstRun)
        {
            /* TODO wait for good mclk instead of delay */
            /* No delay for DFU modes */
            if ((curSamFreq != AUDIO_REBOOT_FROM_DFU) && (curSamFreq != AUDIO_STOP_FOR_DFU) && command)
            {
#if 0
                /* User should ensure MCLK is stable in AudioHwConfig */
                if(retVal1 == SET_SAMPLE_FREQ)
                {
                    timer t;
                    unsigned time;
                    t :> time;
                    t when timerafter(time+AUDIO_PLL_LOCK_DELAY) :> void;
                }
#endif
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

                command = deliver(c_mix_out,
#ifdef SPDIF
                   c_spdif_out,
#else
                   null,
#endif
                   divide, curSamFreq,
#if defined (ADAT_RX) || defined (SPDIF_RX)
                   c_dig_rx,
#endif
                   c);

                if(command == SET_SAMPLE_FREQ)
                {
                    curSamFreq = inuint(c_mix_out);
                }
                else if(command == SET_STREAM_FORMAT_OUT)
                {
                    /* Off = 0
                     * DOP = 1
                     * Native = 2
                     */
                    dsdMode = inuint(c_mix_out);
                    curSamRes_DAC = inuint(c_mix_out);
                }

                /* Currently no more audio will happen after this point */
                if (curSamFreq == AUDIO_STOP_FOR_DFU)
				{
                  	outct(c_mix_out, XS1_CT_END);

                  	while (1)
					{

                    	command = dummy_deliver(c_mix_out);
                        curSamFreq = inuint(c_mix_out);

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

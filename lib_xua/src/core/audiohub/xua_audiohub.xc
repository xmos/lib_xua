/**
 * @file xua_audiohub.xc
 * @brief XMOS USB 2.0 Audio Reference Design.  Audio Functions.
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
#include <string.h>

#include "xua.h"

#include "audioports.h"
#include "mic_array_conf.h"
#if (XUA_SPDIF_TX_EN)
#include "SpdifTransmit.h"
#endif
#ifdef ADAT_TX
#include "adat_tx.h"
#ifndef ADAT_TX_USE_SHARED_BUFF
#error Designed for ADAT tx shared buffer mode ONLY
#endif
#endif

#include "xua_commands.h"
#include "xc_ptr.h"

#define MAX(x,y) ((x)>(y) ? (x) : (y))

static unsigned samplesOut[MAX(NUM_USB_CHAN_OUT, I2S_CHANS_DAC)];

#ifndef ADAT_RX
#define ADAT_RX 0
#endif

#ifndef SPDIF_RX
#define SPDIF_RX 0
#endif

/* Two buffers for ADC data to allow for DAC and ADC I2S ports being offset */
#define IN_CHAN_COUNT (I2S_CHANS_ADC + NUM_PDM_MICS + (8*ADAT_RX) + (2*SPDIF_RX))

static unsigned samplesIn[2][MAX(NUM_USB_CHAN_IN, IN_CHAN_COUNT)];

#if defined(ADAT_RX) && (ADAT_RX ==0)
#undef ADAT_RX
#endif

#if defined(SPDIF_RX) && (SPDIF_RX ==0)
#undef SPDIF_RX
#endif

#if (AUD_TO_USB_RATIO > 1)
#include "src.h"
#endif

#if (DSD_CHANS_DAC != 0)
extern buffered out port:32 p_dsd_dac[DSD_CHANS_DAC];
extern buffered out port:32 p_dsd_clk;
#endif

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
#if CODEC_MASTER
extern buffered in port:32 p_lrclk;
extern buffered in port:32 p_bclk;
#else
extern buffered out port:32 p_lrclk;
extern buffered out port:32 p_bclk;
#endif

unsigned dsdMode = DSD_MODE_OFF;

/* Master clock input */
extern unsafe port p_mclk_in;
extern in port p_mclk_in2;

#if (XUA_SPDIF_TX_EN)
extern buffered out port:32 p_spdif_tx;
#endif

#ifdef ADAT_TX
extern buffered out port:32 p_adat_tx;
#endif

extern clock    clk_audio_mclk;
extern clock    clk_audio_bclk;

#if XUA_SPDIF_TX_EN || defined(ADAT_TX)
extern clock    clk_mst_spd;
#endif

//extern void device_reboot(void);

#define MAX_DIVIDE_48 (MCLK_48/MIN_FREQ_48/64)
#define MAX_DIVIDE_44 (MCLK_44/MIN_FREQ_44/64)
#if (MAX_DIVIDE_44 > MAX_DIVIDE_48)
#define MAX_DIVIDE (MAX_DIVIDE_44)
#else
#define MAX_DIVIDE (MAX_DIVIDE_48)
#endif

#ifdef ADAT_TX
unsigned adatCounter = 0;
unsigned adatSamples[8];

#pragma unsafe arrays
static inline void TransferAdatTxSamples(chanend c_adat_out, const unsigned samplesFromHost[], int smux, int handshake)
{

    /* Do some re-arranging for SMUX.. */
    unsafe
    {
        unsigned * unsafe samplesFromHostAdat = &samplesFromHost[ADAT_TX_INDEX];

        /* Note, when smux == 1 this loop just does a straight 1:1 copy */
        //if(smux != 1)
        {
            int adatSampleIndex = adatCounter;
            for(int i = 0; i < (8/smux); i++)
            {
                adatSamples[adatSampleIndex] = samplesFromHostAdat[i];
                adatSampleIndex += smux;
            }
        }
    }

    adatCounter++;

    if(adatCounter == smux)
    {

#ifdef ADAT_TX_USE_SHARED_BUFF
        unsafe
        {
            /* Wait for ADAT core to be done with buffer */
            /* Note, we are "running ahead" of the ADAT core */
            inuint(c_adat_out);

            /* Send buffer pointer over to ADAT core */
            volatile unsigned * unsafe samplePtr = &adatSamples;
            outuint(c_adat_out, (unsigned) samplePtr);
        }
#else
#pragma loop unroll
        for (int i = 0; i < 8; i++)
        {
            outuint(c_adat_out, samplesFromHost[ADAT_TX_INDEX + i]);
        }
#endif
        adatCounter = 0;
    }
}
#endif

#ifndef NO_USB
#pragma unsafe arrays
static inline unsigned DoSampleTransfer(chanend c_out, const int readBuffNo, const unsigned underflowWord)
{
    outuint(c_out, underflowWord);

    /* Check for sample freq change (or other command) or new samples from mixer*/
    if(testct(c_out))
    {
        unsigned command = inct(c_out);
#ifndef CODEC_MASTER
        if(dsdMode == DSD_MODE_OFF)
        {
            // Set clocks low
            p_lrclk <: 0;
            p_bclk <: 0;
        }
        else
        {
#if(DSD_CHANS_DAC != 0)
            /* DSD Clock might not be shared with lrclk or bclk... */
            p_dsd_clk <: 0;
#endif
        }
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
#if NUM_USB_CHAN_OUT > 0
#pragma loop unroll
        for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
        {
            int tmp = inuint(c_out);
            samplesOut[i] = tmp;
        }
#else
        inuint(c_out);
#endif
        UserBufferManagement(samplesOut, samplesIn[readBuffNo]);

#if NUM_USB_CHAN_IN > 0
#pragma loop unroll
        for(int i = 0; i < NUM_USB_CHAN_IN; i++)
        {
            outuint(c_out, samplesIn[readBuffNo][i]);
        }
#endif
    }

    return 0;
}

#else /* NO_USB */
#pragma unsafe arrays
static inline unsigned DoSampleTransfer(chanend ?c_out, const int readBuffNo, const unsigned underflowWord)
{
    UserBufferManagement(samplesOut, samplesIn[readBuffNo]);
    return 0;
}
#endif /* NO_USB */


#if (DSD_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT > 0)
/* This function performs the DSD native loop and outputs a 32b DSD stream per loop */
static inline void DoDsdNative(unsigned samplesOut[], unsigned &dsdSample_l, unsigned &dsdSample_r, unsigned divide)
{
     /* 8 bits per chan, 1st 1-bit sample in MSB */
    dsdSample_l =  samplesOut[0];
    dsdSample_r =  samplesOut[1];
    dsdSample_r = bitrev(byterev(dsdSample_r));
    dsdSample_l = bitrev(byterev(dsdSample_l));

    asm volatile("out res[%0], %1"::"r"(p_dsd_dac[0]),"r"(dsdSample_l));
    asm volatile("out res[%0], %1"::"r"(p_dsd_dac[1]),"r"(dsdSample_r));
}

/* This function performs the DOP loop and collects 16b of DSD per loop 
   and outputs a 32b word into the port buffer every other cycle. */
static inline void DoDsdDop(int &everyOther, unsigned samplesOut[], unsigned &dsdSample_l, unsigned &dsdSample_r, unsigned divide)
{
    if(!everyOther)
    {
        dsdSample_l = ((samplesOut[0] & 0xffff00) << 8);
        dsdSample_r = ((samplesOut[1] & 0xffff00) << 8);
        everyOther = 1;
    }
    else 
    {
        everyOther = 0;
        dsdSample_l =  dsdSample_l | ((samplesOut[0] & 0xffff00) >> 8);
        dsdSample_r =  dsdSample_r | ((samplesOut[1] & 0xffff00) >> 8);

        asm volatile("out res[%0], %1"::"r"(p_dsd_dac[0]),"r"(bitrev(dsdSample_l)));
        asm volatile("out res[%0], %1"::"r"(p_dsd_dac[1]),"r"(bitrev(dsdSample_r)));
    }
}

/* When DSD is enabled and streaming is standard PCM, this function checks for a series of DoP markers in the upper byte.
   If found it will exit deliver() with the command to restart in DoP mode.
   When in DoP mode, this function will check for a single absence of the DoP marker and exit deliver() with the command
   to restart in I2S mode. */
static inline int DoDsdDopCheck(unsigned &dsdMode, int &dsdCount, unsigned curSamFreq, unsigned samplesOut[], unsigned &dsdMarker)
{
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
    else if(dsdMode == DSD_MODE_DOP) 
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
    return 1;
}
#endif

#if !CODEC_MASTER
static inline void InitPorts_master(unsigned divide)
{
    unsigned tmp;
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

#if (DSD_CHANS_DAC > 0)
    } /* if (!dsdMode) */
    else
    {
        /* p_dsd_clk must start high */
        p_dsd_clk <: 0x80000000;
    }
#endif
}
#endif

#if CODEC_MASTER
static inline void InitPorts_slave(unsigned divide)
{
    unsigned tmp;

    /* Wait for LRCLK edge (in I2S LRCLK = 0 is left, TDM rising edge is start of frame) */
    p_lrclk when pinseq(0) :> void;
    p_lrclk when pinseq(1) :> void;
    p_lrclk when pinseq(0) :> void;
    p_lrclk when pinseq(1) :> void;
#if I2S_MODE_TDM
    p_lrclk when pinseq(0) :> void;
    p_lrclk when pinseq(1) :> void @ tmp;
#else
    p_lrclk when pinseq(0) :> void @ tmp;
#endif

    tmp += (I2S_CHANS_PER_FRAME * 32) - 32 + 1 ;
    /* E.g. 2 * 32 - 32 + 1 = 33 for stereo */
    /* E.g. 8 * 32 - 32 + 1 = 225 for 8 chan TDM */

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
       asm("setpt res[%0], %1"::"r"(p_i2s_adc[i]),"r"(tmp-1));
    }
#endif

    asm("setpt res[%0], %1"::"r"(p_lrclk),"r"(tmp-1));
}
#endif



#if (CODEC_MASTER == 0)
#pragma unsafe arrays
unsigned static deliver_master(chanend ?c_out, chanend ?c_spd_out
#ifdef ADAT_TX
    , chanend c_adat_out
    , unsigned adatSmuxMode
#endif
    , unsigned divide, unsigned curSamFreq
#if(defined(SPDIF_RX) || defined(ADAT_RX))
    , chanend c_dig_rx
#endif
#if (NUM_PDM_MICS > 0)
    , chanend c_pdm_pcm
#endif
)
{
    /* Since DAC and ADC buffered ports off by one sample we buffer previous ADC frame */
    unsigned readBuffNo = 0;
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
#ifdef ADAT_TX
    adatCounter = 0;
#endif

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

    unsigned audioToUsbRatioCounter = 0;
#if (NUM_PDM_MICS > 0)
    unsigned audioToMicsRatioCounter = 0;
#endif

#if (AUD_TO_USB_RATIO > 1)
    union i2sInDs3
    {
        long long doubleWordAlignmentEnsured;
        int32_t delayLine[I2S_DOWNSAMPLE_CHANS_IN][SRC_FF3V_FIR_NUM_PHASES][SRC_FF3V_FIR_TAPS_PER_PHASE];
    } i2sInDs3;
    memset(&i2sInDs3.delayLine, 0, sizeof i2sInDs3.delayLine);
    int64_t i2sInDs3Sum[I2S_DOWNSAMPLE_CHANS_IN];

    union i2sOutUs3
    {
        long long doubleWordAlignmentEnsured;
        int32_t delayLine[I2S_CHANS_DAC][SRC_FF3V_FIR_TAPS_PER_PHASE];
    } i2sOutUs3;
    memset(&i2sOutUs3.delayLine, 0, sizeof i2sOutUs3.delayLine);
#endif /* (AUD_TO_USB_RATIO > 1) */


#if ((DEBUG_MIC_ARRAY == 1) && (NUM_PDM_MICS > 0))
    /* Get initial samples from PDM->PCM converter to avoid stalling the decimators */
    c_pdm_pcm <: 1;
    master
    {
#pragma loop unroll
        for(int i = PDM_MIC_INDEX; i < (NUM_PDM_MICS + PDM_MIC_INDEX); i++)
        {
            c_pdm_pcm :> samplesIn[readBuffNo][i];
        }
    }
#endif // ((DEBUG_MIC_ARRAY == 1) && (NUM_PDM_MICS > 0))

    UserBufferManagementInit();

    unsigned command = DoSampleTransfer(c_out, readBuffNo, underflowWord);

    // Reinitialise user state before entering the main loop
    UserBufferManagementInit();

#ifdef ADAT_TX
    unsafe{
    //TransferAdatTxSamples(c_adat_out, samplesOut, adatSmuxMode, 0);
    volatile unsigned * unsafe samplePtr = &samplesOut[ADAT_TX_INDEX];
    outuint(c_adat_out, (unsigned) samplePtr);
    }
#endif
    if(command)
    {
        return command;
    }

    InitPorts_master(divide);

    /* Main Audio I/O loop */
    while (1)
    {
        {
#if (DSD_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT > 0)
            if(dsdMode == DSD_MODE_NATIVE) 
                DoDsdNative(samplesOut, dsdSample_l, dsdSample_r, divide);
            else if(dsdMode == DSD_MODE_DOP) 
                DoDsdDop(everyOther, samplesOut, dsdSample_l, dsdSample_r, divide);
            else
#endif
            {
#if (I2S_CHANS_ADC != 0)
#if (AUD_TO_USB_RATIO > 1)
                if (0 == audioToUsbRatioCounter)
                {
                    memset(&i2sInDs3Sum, 0, sizeof i2sInDs3Sum);
                }
#endif /* (AUD_TO_USB_RATIO > 1) */
                /* Input previous L sample into L in buffer */
                index = 0;
                /* First input (i.e. frameCount == 0) we read last ADC channel of previous frame.. */
                unsigned buffIndex = (frameCount > 1) ? !readBuffNo : readBuffNo;

#pragma loop unroll
                /* First time around we get channel 7 of TDM8 */
                for(int i = 0; i < I2S_CHANS_ADC; i+=I2S_CHANS_PER_FRAME)
                {
                    // p_i2s_adc[index++] :> sample;
                    // Manual IN instruction since compiler generates an extra setc per IN (bug #15256)
                    unsigned sample;
                    asm volatile("in %0, res[%1]" : "=r"(sample)  : "r"(p_i2s_adc[index++]));
                    sample = bitrev(sample);
                    int chanIndex = ((frameCount-2)&(I2S_CHANS_PER_FRAME-1))+i; // channels 0, 2, 4.. on each line.

#if (AUD_TO_USB_RATIO > 1)
                    if ((AUD_TO_USB_RATIO - 1) == audioToUsbRatioCounter)
                    {
                        samplesIn[buffIndex][chanIndex] =
                            src_ds3_voice_add_final_sample(
                                i2sInDs3Sum[chanIndex],
                                i2sInDs3.delayLine[chanIndex][audioToUsbRatioCounter],
                                src_ff3v_fir_coefs[audioToUsbRatioCounter],
                                sample);
                    }
                    else
                    {
                        i2sInDs3Sum[chanIndex] =
                            src_ds3_voice_add_sample(
                                i2sInDs3Sum[chanIndex],
                                i2sInDs3.delayLine[chanIndex][audioToUsbRatioCounter],
                                src_ff3v_fir_coefs[audioToUsbRatioCounter],
                                sample);
                    }
#else
                    samplesIn[buffIndex][chanIndex] = sample;
#endif /* (AUD_TO_USB_RATIO > 1) */
                }
#endif

                /* LR clock delayed by one clock, This is so MSB is output on the falling edge of BCLK
                 * after the falling edge on which LRCLK was toggled. (see I2S spec) */
                /* Generate clocks LR Clock low - LEFT */
#if I2S_MODE_TDM
                p_lrclk <: 0x00000000;
#else
                p_lrclk <: 0x80000000;
#endif

#pragma xta endpoint "i2s_output_l"

#if (I2S_CHANS_DAC != 0)
                index = 0;
#pragma loop unroll
                /* Output "even" channel to DAC (i.e. left) */
                for(int i = 0; i < I2S_CHANS_DAC; i+=I2S_CHANS_PER_FRAME)
                {
#if (AUD_TO_USB_RATIO > 1)
                    if (0 == audioToUsbRatioCounter)
                    {
                        samplesOut[frameCount+i] = src_us3_voice_input_sample(i2sOutUs3.delayLine[i],
                                                                              src_ff3v_fir_coefs[2],
                                                                              samplesOut[frameCount+i]);
                    }
                    else /* audioToUsbRatioCounter == 1 or 2 */
                    {
                        samplesOut[frameCount+i] = src_us3_voice_get_next_sample(i2sOutUs3.delayLine[i],
                                                                                 src_ff3v_fir_coefs[2-audioToUsbRatioCounter]);
                    }
#endif /* (AUD_TO_USB_RATIO > 1) */
                    p_i2s_dac[index++] <: bitrev(samplesOut[frameCount +i]);
                }
#endif // (I2S_CHANS_DAC != 0)

#ifdef ADAT_TX
                 TransferAdatTxSamples(c_adat_out, samplesOut, adatSmuxMode, 1);
#endif

            if(frameCount == 0)
            {

#if defined(SPDIF_RX) || defined(ADAT_RX)
                /* Sync with clockgen */
                inuint(c_dig_rx);

                /* Note, digi-data we just store in samplesIn[readBuffNo] - we only double buffer the I2S input data */
#endif
#ifdef SPDIF_RX
                asm("ldw %0, dp[g_digData]"  :"=r"(samplesIn[readBuffNo][SPDIF_RX_INDEX + 0]));
                asm("ldw %0, dp[g_digData+4]":"=r"(samplesIn[readBuffNo][SPDIF_RX_INDEX + 1]));
#endif
#ifdef ADAT_RX
                asm("ldw %0, dp[g_digData+8]" :"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX]));
                asm("ldw %0, dp[g_digData+12]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 1]));
                asm("ldw %0, dp[g_digData+16]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 2]));
                asm("ldw %0, dp[g_digData+20]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 3]));
                asm("ldw %0, dp[g_digData+24]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 4]));
                asm("ldw %0, dp[g_digData+28]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 5]));
                asm("ldw %0, dp[g_digData+32]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 6]));
                asm("ldw %0, dp[g_digData+36]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 7]));
#endif

#if defined(SPDIF_RX) || defined(ADAT_RX)
                /* Request digital data (with prefill) */
                outuint(c_dig_rx, 0);
#endif
#if (XUA_SPDIF_TX_EN) && (NUM_USB_CHAN_OUT > 0)
                outuint(c_spd_out, samplesOut[SPDIF_TX_INDEX]);  /* Forward sample to S/PDIF Tx thread */
                unsigned sample = samplesOut[SPDIF_TX_INDEX + 1];
                outuint(c_spd_out, sample);                      /* Forward sample to S/PDIF Tx thread */
#endif

#if (NUM_PDM_MICS > 0)
                if ((AUD_TO_MICS_RATIO - 1) == audioToMicsRatioCounter)
                {
                    /* Get samples from PDM->PCM converter */
                    c_pdm_pcm <: 1;
                    master
                    {
#pragma loop unroll
                        for(int i = PDM_MIC_INDEX; i < (NUM_PDM_MICS + PDM_MIC_INDEX); i++)
                        {
                            c_pdm_pcm :> samplesIn[readBuffNo][i];
                        }
                    }
                    audioToMicsRatioCounter = 0;
                }
                else
                {
                    ++audioToMicsRatioCounter;
                }
#endif
            }

#if (I2S_CHANS_ADC != 0)
                index = 0;
                /* Channels 0, 2, 4.. on each line */
#pragma loop unroll
                for(int i = 0; i < I2S_CHANS_ADC; i += I2S_CHANS_PER_FRAME)
                {
                    /* Manual IN instruction since compiler generates an extra setc per IN (bug #15256) */
                    unsigned sample;
                    asm volatile("in %0, res[%1]" : "=r"(sample)  : "r"(p_i2s_adc[index++]));
                    sample = bitrev(sample);
                    int chanIndex = ((frameCount-1)&(I2S_CHANS_PER_FRAME-1))+i; // channels 1, 3, 5.. on each line.
#if (AUD_TO_USB_RATIO > 1 && !I2S_DOWNSAMPLE_MONO_IN)
                    if ((AUD_TO_USB_RATIO - 1) == audioToUsbRatioCounter)
                    {
                        samplesIn[buffIndex][chanIndex] =
                            src_ds3_voice_add_final_sample(
                                i2sInDs3Sum[chanIndex],
                                i2sInDs3.delayLine[chanIndex][audioToUsbRatioCounter],
                                src_ff3v_fir_coefs[audioToUsbRatioCounter],
                                sample);
                    }
                    else
                    {
                        i2sInDs3Sum[chanIndex] =
                            src_ds3_voice_add_sample(
                                i2sInDs3Sum[chanIndex],
                                i2sInDs3.delayLine[chanIndex][audioToUsbRatioCounter],
                                src_ff3v_fir_coefs[audioToUsbRatioCounter],
                                sample);
                    }
#else
                    samplesIn[buffIndex][chanIndex] = sample;
#endif /* (AUD_TO_USB_RATIO > 1) && !I2S_DOWNSAMPLE_MONO_IN */
                }
#endif

#if I2S_MODE_TDM
                if(frameCount == (I2S_CHANS_PER_FRAME-2))
                    p_lrclk <: 0x80000000;
                else
                   p_lrclk <: 0x00000000;
#else
                p_lrclk <: 0x7FFFFFFF;
#endif

                index = 0;
#pragma xta endpoint "i2s_output_r"
#if (I2S_CHANS_DAC != 0)
                /* Output "odd" channel to DAC (i.e. right) */
#pragma loop unroll
                for(int i = 1; i < I2S_CHANS_DAC; i+=I2S_CHANS_PER_FRAME)
                {
#if (AUD_TO_USB_RATIO > 1)
                    if (audioToUsbRatioCounter == 0)
                    {
                        samplesOut[frameCount+i] = src_us3_voice_input_sample(i2sOutUs3.delayLine[i],
                                                                              src_ff3v_fir_coefs[2],
                                                                              samplesOut[frameCount+i]);
                    }
                    else
                    { /* audioToUsbRatioCounter is 1 or 2 */
                        samplesOut[frameCount+i] = src_us3_voice_get_next_sample(i2sOutUs3.delayLine[i],
                                                                                 src_ff3v_fir_coefs[2-audioToUsbRatioCounter]);
                    }
#endif /* (AUD_TO_USB_RATIO > 1) */
                    p_i2s_dac[index++] <: bitrev(samplesOut[frameCount + i]);
                }
#endif // (I2S_CHANS_DAC != 0)

            }  // !dsdMode


#if (DSD_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT > 0)
            if(DoDsdDopCheck(dsdMode, dsdCount, curSamFreq, samplesOut, dsdMarker) == 0)
                return 0;
#endif

#if I2S_MODE_TDM
            /* Increase frameCount by 2 since we have output two channels (per data line) */
            frameCount+=2;
            if(frameCount == I2S_CHANS_PER_FRAME)
#endif
            {
                if ((AUD_TO_USB_RATIO - 1) == audioToUsbRatioCounter)
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
                    audioToUsbRatioCounter = 0;
                    frameCount = 0;
                    readBuffNo = !readBuffNo;
                }
                else
                {
                    ++audioToUsbRatioCounter;
                }
            }
        }
    }
#pragma xta endpoint "deliver_return"
    return 0;
}
#endif


#if (CODEC_MASTER == 1)

/* I2S delivery thread */
#pragma unsafe arrays
unsigned static deliver_slave(chanend ?c_out, chanend ?c_spd_out
#ifdef ADAT_TX
    , chanend c_adat_out
    , unsigned adatSmuxMode
#endif
    , unsigned divide, unsigned curSamFreq
#if(defined(SPDIF_RX) || defined(ADAT_RX))
    , chanend c_dig_rx
#endif
#if (NUM_PDM_MICS > 0)
    , chanend c_pdm_pcm
#endif
)
{
    /* Since DAC and ADC buffered ports off by one sample we buffer previous ADC frame */
    unsigned readBuffNo = 0;
    unsigned index;

#ifdef RAMP_CHECK
    unsigned prev=0;
    int started = 0;
#endif

    int firstIteration = 1;
    unsigned underflowWord = 0;
    unsigned frameCount = 0;
#ifdef ADAT_TX
    adatCounter = 0;
#endif

    unsigned audioToUsbRatioCounter = 0;
#if (NUM_PDM_MICS > 0)
    unsigned audioToMicsRatioCounter = 0;
#endif

#if (AUD_TO_USB_RATIO > 1)
    union i2sInDs3
    {
        long long doubleWordAlignmentEnsured;
        int32_t delayLine[I2S_DOWNSAMPLE_CHANS_IN][SRC_FF3V_FIR_NUM_PHASES][SRC_FF3V_FIR_TAPS_PER_PHASE];
    } i2sInDs3;
    memset(&i2sInDs3.delayLine, 0, sizeof i2sInDs3.delayLine);
    int64_t i2sInDs3Sum[I2S_DOWNSAMPLE_CHANS_IN];

    union i2sOutUs3
    {
        long long doubleWordAlignmentEnsured;
        int32_t delayLine[I2S_CHANS_DAC][SRC_FF3V_FIR_TAPS_PER_PHASE];
    } i2sOutUs3;
    memset(&i2sOutUs3.delayLine, 0, sizeof i2sOutUs3.delayLine);
#endif /* (AUD_TO_USB_RATIO > 1) */


#if ((DEBUG_MIC_ARRAY == 1) && (NUM_PDM_MICS > 0))
    /* Get initial samples from PDM->PCM converter to avoid stalling the decimators */
    c_pdm_pcm <: 1;
    master
    {
#pragma loop unroll
        for(int i = PDM_MIC_INDEX; i < (NUM_PDM_MICS + PDM_MIC_INDEX); i++)
        {
            c_pdm_pcm :> samplesIn[readBuffNo][i];
        }
    }
#endif // ((DEBUG_MIC_ARRAY == 1) && (NUM_PDM_MICS > 0))

    UserBufferManagementInit();

    unsigned command = DoSampleTransfer(c_out, readBuffNo, underflowWord);

    // Reinitialise user state before entering the main loop
    UserBufferManagementInit();

#ifdef ADAT_TX
    unsafe{
    //TransferAdatTxSamples(c_adat_out, samplesOut, adatSmuxMode, 0);
    volatile unsigned * unsafe samplePtr = &samplesOut[ADAT_TX_INDEX];
    outuint(c_adat_out, (unsigned) samplePtr);
    }
#endif
    if(command)
    {
        return command;
    }

    /* Main Audio I/O loop */
    while (1)
    {
        /* In CODEC master mode, the I/O loop assumes L/RCLK = 32bit clocks.
         * Check this every iteration and resync if we get a bclk glitch.
         */
        unsigned lrval;
        unsigned syncError = 0;

        InitPorts_slave(divide);

        while (!syncError)
        {
#if (I2S_CHANS_ADC != 0)
#if (AUD_TO_USB_RATIO > 1)
            if (0 == audioToUsbRatioCounter)
            {
                memset(&i2sInDs3Sum, 0, sizeof i2sInDs3Sum);
            }
#endif /* (AUD_TO_USB_RATIO > 1) */
            /* Input previous L sample into L in buffer */
            index = 0;
            /* First input (i.e. frameCount == 0) we read last ADC channel of previous frame.. */
            unsigned buffIndex = (frameCount > 1) ? !readBuffNo : readBuffNo;


            #pragma loop unroll
            /* First time around we get channel 7 of TDM8 */
            for(int i = 0; i < I2S_CHANS_ADC; i+=I2S_CHANS_PER_FRAME)
            {
                // p_i2s_adc[index++] :> sample;
                // Manual IN instruction since compiler generates an extra setc per IN (bug #15256)
                unsigned sample;
                asm volatile("in %0, res[%1]" : "=r"(sample)  : "r"(p_i2s_adc[index++]));
                
                sample = bitrev(sample);
                int chanIndex = ((frameCount-2)&(I2S_CHANS_PER_FRAME-1))+i; // channels 0, 2, 4.. on each line.
#if (AUD_TO_USB_RATIO > 1)
                if ((AUD_TO_USB_RATIO - 1) == audioToUsbRatioCounter)
                {
                    samplesIn[buffIndex][chanIndex] =
                        src_ds3_voice_add_final_sample(
                            i2sInDs3Sum[chanIndex],
                            i2sInDs3.delayLine[chanIndex][audioToUsbRatioCounter],
                            src_ff3v_fir_coefs[audioToUsbRatioCounter],
                            sample);
                }
                else
                {
                    i2sInDs3Sum[chanIndex] =
                        src_ds3_voice_add_sample(
                            i2sInDs3Sum[chanIndex],
                            i2sInDs3.delayLine[chanIndex][audioToUsbRatioCounter],
                            src_ff3v_fir_coefs[audioToUsbRatioCounter],
                            sample);
                }
#else
                samplesIn[buffIndex][chanIndex] = sample;
#endif /* (AUD_TO_USB_RATIO > 1) */
            }
#endif //(I2S_CHANS_ADC != 0)

            /* LR Clock sync check */
            p_lrclk :> lrval;
                   
            if(I2S_MODE_TDM)
            {        
                /* Only check for the rising edge of frame sync being in the right place because falling edge timing not specified */
                if (frameCount == (I2S_CHANS_PER_FRAME-1)) 
                {
                    lrval &= 0xc0000000;                 // Mask off last two (MSB) frame clock bits which are the most recently sampled
                    syncError += (lrval != 0x80000000);  // We need MSB = 1 and MSB-1 = 0 to signify rising edge
                }
            }
            else
            {
                syncError += (lrval != 0x80000000);
            }

#pragma xta endpoint "i2s_output_l"

#if (I2S_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT != 0)
            index = 0;
#pragma loop unroll
            /* Output "even" channel to DAC (i.e. left) */
            for(int i = 0; i < I2S_CHANS_DAC; i+=I2S_CHANS_PER_FRAME)
            {
#if (AUD_TO_USB_RATIO > 1)
                if (0 == audioToUsbRatioCounter)
                {
                    samplesOut[frameCount+i] = src_us3_voice_input_sample(i2sOutUs3.delayLine[i],
                                                                          src_ff3v_fir_coefs[2],
                                                                          samplesOut[frameCount+i]);
                }
                else /* audioToUsbRatioCounter == 1 or 2 */
                {
                    samplesOut[frameCount+i] = src_us3_voice_get_next_sample(i2sOutUs3.delayLine[i],
                                                                             src_ff3v_fir_coefs[2-audioToUsbRatioCounter]);
                }
#endif /* (AUD_TO_USB_RATIO > 1) */
                p_i2s_dac[index++] <: bitrev(samplesOut[frameCount +i]);
            }
#endif // (I2S_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT != 0)

#ifdef ADAT_TX
             TransferAdatTxSamples(c_adat_out, samplesOut, adatSmuxMode, 1);
#endif

        if(frameCount == 0)
        {

#if defined(SPDIF_RX) || defined(ADAT_RX)
            /* Sync with clockgen */
            inuint(c_dig_rx);

            /* Note, digi-data we just store in samplesIn[readBuffNo] - we only double buffer the I2S input data */
#endif
#ifdef SPDIF_RX
            asm("ldw %0, dp[g_digData]"  :"=r"(samplesIn[readBuffNo][SPDIF_RX_INDEX + 0]));
            asm("ldw %0, dp[g_digData+4]":"=r"(samplesIn[readBuffNo][SPDIF_RX_INDEX + 1]));
#endif
#ifdef ADAT_RX
            asm("ldw %0, dp[g_digData+8]" :"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX]));
            asm("ldw %0, dp[g_digData+12]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 1]));
            asm("ldw %0, dp[g_digData+16]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 2]));
            asm("ldw %0, dp[g_digData+20]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 3]));
            asm("ldw %0, dp[g_digData+24]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 4]));
            asm("ldw %0, dp[g_digData+28]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 5]));
            asm("ldw %0, dp[g_digData+32]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 6]));
            asm("ldw %0, dp[g_digData+36]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 7]));
#endif

#if defined(SPDIF_RX) || defined(ADAT_RX)
            /* Request digital data (with prefill) */
            outuint(c_dig_rx, 0);
#endif
#if ((XUA_SPDIF_TX_EN) && (NUM_USB_CHAN_OUT > 0))
            outuint(c_spd_out, samplesOut[SPDIF_TX_INDEX]);  /* Forward sample to S/PDIF Tx thread */
            unsigned sample = samplesOut[SPDIF_TX_INDEX + 1];
            outuint(c_spd_out, sample);                      /* Forward sample to S/PDIF Tx thread */
#endif

#if (NUM_PDM_MICS > 0)
            if ((AUD_TO_MICS_RATIO - 1) == audioToMicsRatioCounter)
            {
                /* Get samples from PDM->PCM converter */
                c_pdm_pcm <: 1;
                master
                {
#pragma loop unroll
                    for(int i = PDM_MIC_INDEX; i < (NUM_PDM_MICS + PDM_MIC_INDEX); i++)
                    {
                        c_pdm_pcm :> samplesIn[readBuffNo][i];
                    }
                }
                audioToMicsRatioCounter = 0;
            }
            else
            {
                ++audioToMicsRatioCounter;
            }
#endif
        }

#if (I2S_CHANS_ADC != 0)
            index = 0;
            /* Channels 0, 2, 4.. on each line */
#pragma loop unroll
            for(int i = 0; i < I2S_CHANS_ADC; i += I2S_CHANS_PER_FRAME)
            {
                /* Manual IN instruction since compiler generates an extra setc per IN (bug #15256) */
                unsigned sample;
                asm volatile("in %0, res[%1]" : "=r"(sample)  : "r"(p_i2s_adc[index++]));
                sample = bitrev(sample);
                int chanIndex = ((frameCount-1)&(I2S_CHANS_PER_FRAME-1))+i; // channels 1, 3, 5.. on each line.
#if (AUD_TO_USB_RATIO > 1 && !I2S_DOWNSAMPLE_MONO_IN)
                if ((AUD_TO_USB_RATIO - 1) == audioToUsbRatioCounter)
                {
                    samplesIn[buffIndex][chanIndex] =
                        src_ds3_voice_add_final_sample(
                            i2sInDs3Sum[chanIndex],
                            i2sInDs3.delayLine[chanIndex][audioToUsbRatioCounter],
                            src_ff3v_fir_coefs[audioToUsbRatioCounter],
                            sample);
                }
                else
                {
                    i2sInDs3Sum[chanIndex] =
                        src_ds3_voice_add_sample(
                            i2sInDs3Sum[chanIndex],
                            i2sInDs3.delayLine[chanIndex][audioToUsbRatioCounter],
                            src_ff3v_fir_coefs[audioToUsbRatioCounter],
                            sample);
                }
#else
                samplesIn[buffIndex][chanIndex] = sample;
#endif /* (AUD_TO_USB_RATIO > 1) && !I2S_DOWNSAMPLE_MONO_IN */
            }
#endif //(I2S_CHANS_ADC != 0)

            /* LR Clock sync check */
            p_lrclk :> lrval;

            if(I2S_MODE_TDM)
            {
                /* Do nothing */
                // We do not check this part of the frame because TDM frame sync falling egde timing
                // is not defined. We only care about rising edge which is checked in first half of frame
            }
            else
            {  
                syncError += (lrval != 0x7fffffff);
            }

            index = 0;
#pragma xta endpoint "i2s_output_r"
#if (I2S_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT != 0)
            /* Output "odd" channel to DAC (i.e. right) */
#pragma loop unroll
            for(int i = 1; i < I2S_CHANS_DAC; i+=I2S_CHANS_PER_FRAME)
            {
#if (AUD_TO_USB_RATIO > 1)
                if (audioToUsbRatioCounter == 0)
                {
                    samplesOut[frameCount+i] = src_us3_voice_input_sample(i2sOutUs3.delayLine[i],
                                                                          src_ff3v_fir_coefs[2],
                                                                          samplesOut[frameCount+i]);
                }
                else
                { /* audioToUsbRatioCounter is 1 or 2 */
                    samplesOut[frameCount+i] = src_us3_voice_get_next_sample(i2sOutUs3.delayLine[i],
                                                                             src_ff3v_fir_coefs[2-audioToUsbRatioCounter]);
                }
#endif /* (AUD_TO_USB_RATIO > 1) */
                p_i2s_dac[index++] <: bitrev(samplesOut[frameCount + i]);
            }
#endif // (I2S_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT != 0)


#if I2S_MODE_TDM
            /* Increase frameCount by 2 since we have output two channels (per data line) */
            frameCount+=2;
            if(frameCount == I2S_CHANS_PER_FRAME)
#endif
            {
                if ((AUD_TO_USB_RATIO - 1) == audioToUsbRatioCounter)
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
                    audioToUsbRatioCounter = 0;
                    frameCount = 0;
                    readBuffNo = !readBuffNo;
                }
                else
                {
                    ++audioToUsbRatioCounter;
                }
            }
        }
    }
#pragma xta endpoint "deliver_return"
    return 0;
}
#endif //CODEC_MASTER

#if (XUA_SPDIF_TX_EN) && (SPDIF_TX_TILE != AUDIO_IO_TILE)
void SpdifTxWrapper(chanend c_spdif_tx)
{
    unsigned portId;
    //configure_clock_src(clk, p_mclk);

    // TODO could share clock block here..
    // NOTE, Assuming SPDIF tile == USB tile here..
    asm("ldw %0, dp[p_mclk_in2]":"=r"(portId));
    asm("setclk res[%0], %1"::"r"(clk_mst_spd), "r"(portId));
    configure_out_port_no_ready(p_spdif_tx, clk_mst_spd, 0);
    set_clock_fall_delay(clk_mst_spd, 7);
    start_clock(clk_mst_spd);

    while(1)
    {
        SpdifTransmit(p_spdif_tx, c_spdif_tx);
    }
}
#endif

#if XUA_DFU_EN
[[distributable]]
void DFUHandler(server interface i_dfu i, chanend ?c_user_cmd);
#endif

/* This function is a dummy version of the deliver thread that does not
   connect to the codec ports. It is used during DFU reset. */

#pragma select handler
void testct_byref(chanend c, int &returnVal)
{
    returnVal = 0;
    if(testct(c))
        returnVal = 1;
}

#if (XUA_DFU_EN == 1)
[[combinable]]
static void dummy_deliver(chanend ?c_out, unsigned &command)
{
    int ct;


    while (1)
    {
        select
        {
            /* Check for sample freq change or new samples from mixer*/
            case testct_byref(c_out, ct):
                if(ct)
                {
                    unsigned command = inct(c_out);
                    return;
                }
                else
                {

#if NUM_USB_CHAN_OUT > 0
#pragma loop unroll
                    for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
                    {
                        int tmp = inuint(c_out);
                        samplesOut[i] = tmp;
                    }
#else
                    inuint(c_out);
#endif

#if NUM_USB_CHAN_IN > 0
#pragma loop unroll
                    for(int i = 0; i < NUM_USB_CHAN_IN; i++)
                    {
                        outuint(c_out, 0);
                    }
#endif
                }

                outuint(c_out, 0);
            break;
        }
    }
}
#endif

#if XUA_DFU_EN
 [[distributable]]
 void DFUHandler(server interface i_dfu i, chanend ?c_user_cmd);
 #endif

void XUA_AudioHub(chanend ?c_mix_out
#if (XUA_SPDIF_TX_EN) && (SPDIF_TX_TILE != AUDIO_IO_TILE)
    , chanend c_spdif_out
#endif
#if (defined(ADAT_RX) || defined(SPDIF_RX))
    , chanend c_dig_rx
#endif
#if (XUD_TILE != 0) && (AUDIO_IO_TILE == 0) && (XUA_DFU_EN == 1)
    , server interface i_dfu ?dfuInterface
#endif
#if (NUM_PDM_MICS > 0)
    , chanend c_pdm_in
#endif
)
{
#if (XUA_SPDIF_TX_EN) && (SPDIF_TX_TILE == AUDIO_IO_TILE)
    chan c_spdif_out;
#endif
#ifdef ADAT_TX
    chan c_adat_out;
    unsigned adatSmuxMode = 0;
    unsigned adatMultiple = 0;
#endif

    unsigned curSamFreq = DEFAULT_FREQ * AUD_TO_USB_RATIO;
    unsigned curSamRes_DAC = STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS; /* Default to something reasonable */
    unsigned curSamRes_ADC = STREAM_FORMAT_INPUT_1_RESOLUTION_BITS; /* Default to something reasonable - note, currently this never changes*/
    unsigned command;
    unsigned mClk;
    unsigned divide;
    unsigned firstRun = 1;

    /* Clock master clock-block from master-clock port */
    /* Note, marked unsafe since other cores may be using this mclk port */
    unsafe
    {
        configure_clock_src(clk_audio_mclk, (port) p_mclk_in);
    }

    start_clock(clk_audio_mclk);

#if (DSD_CHANS_DAC > 0)
    /* Make sure the DSD ports are on and buffered - just in case they are not shared with I2S */
    EnableBufferedPort(p_dsd_clk, 32);
    for(int i = 0; i< DSD_CHANS_DAC; i++)
    {
        EnableBufferedPort(p_dsd_dac[i], 32);
    }
#endif
#ifdef ADAT_TX
    /* Share SPDIF clk blk */
    unsafe
    {
        configure_clock_src(clk_mst_spd, (port)p_mclk_in);
    }
    configure_out_port_no_ready(p_adat_tx, clk_mst_spd, 0);
    set_clock_fall_delay(clk_mst_spd, 7);
#if (XUA_SPDIF_TX_EN == 0)
    start_clock(clk_mst_spd);
#endif
#endif
    /* Configure ADAT/SPDIF tx ports */
#if (XUA_SPDIF_TX_EN) && (SPDIF_TX_TILE == AUDIO_IO_TILE)
    unsafe
    {
        SpdifTransmitPortConfig(p_spdif_tx, clk_mst_spd,  (port)p_mclk_in);
    }
#endif

    /* Perform required CODEC/ADC/DAC initialisation */
    AudioHwInit();

    while(1)
    {
        /* Calculate what master clock we should be using */
        if ((MCLK_441 % curSamFreq) == 0)
        {
            mClk = MCLK_441;
#ifdef ADAT_TX
            /* Calculate ADAT SMUX mode (1, 2, 4) */
            adatSmuxMode = curSamFreq / 44100;
            adatMultiple = mClk / 44100;
#endif
        }
        else if ((MCLK_48 % curSamFreq) == 0)
        {
            mClk = MCLK_48;
#ifdef ADAT_TX
            /* Calculate ADAT SMUX mode (1, 2, 4) */
            adatSmuxMode = curSamFreq / 48000;
            adatMultiple = mClk / 48000;
#endif
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

            /* TODO; we should catch and handle the case when divide is 0. Currently design will lock up */
       }


#if (DSD_CHANS_DAC > 0)
        if(dsdMode)
        {
        /* Configure audio ports */
        ConfigAudioPortsWrapper(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
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
                divide, curSamFreq, dsdMode);
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
            divide, curSamFreq, dsdMode);
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
            AudioHwConfig(curFreq, mClk, dsdMode, curSamRes_DAC, curSamRes_ADC);
        }

        if(!firstRun)
        {
            /* TODO wait for good mclk instead of delay */
            /* No delay for DFU modes */
            if (((curSamFreq / AUD_TO_USB_RATIO) != AUDIO_REBOOT_FROM_DFU) && ((curSamFreq / AUD_TO_USB_RATIO) != AUDIO_STOP_FOR_DFU) && command)
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
#ifndef NO_USB
                outct(c_mix_out, XS1_CT_END);
#endif
            }
        }
        firstRun = 0;

        par
        {

#if (XUA_SPDIF_TX_EN) && (SPDIF_TX_TILE == AUDIO_IO_TILE)
            {
                set_thread_fast_mode_on();
                SpdifTransmit(p_spdif_tx, c_spdif_out);
            }
#endif

#ifdef ADAT_TX
            {
                set_thread_fast_mode_on();
                adat_tx_port(c_adat_out, p_adat_tx);
            }
#endif
            {
#if (XUA_SPDIF_TX_EN)
                /* Communicate master clock and sample freq to S/PDIF thread */
                outuint(c_spdif_out, curSamFreq);
                outuint(c_spdif_out, mClk);
#endif

#if NUM_PDM_MICS > 0
                /* Send decimation factor to PDM task(s) */
                c_pdm_in <: curSamFreq / AUD_TO_MICS_RATIO;
#endif

#ifdef ADAT_TX
                // Configure ADAT parameters ...
                //
                // adat_oversampling =  256 for MCLK = 12M288 or 11M2896
                //                   =  512 for MCLK = 24M576 or 22M5792
                //                   = 1024 for MCLK = 49M152 or 45M1584
                //
                // adatSmuxMode   = 1 for FS =  44K1 or  48K0
                //                = 2 for FS =  88K2 or  96K0
                //                = 4 for FS = 176K4 or 192K0
                outuint(c_adat_out, adatMultiple);
                outuint(c_adat_out, adatSmuxMode);
#endif
#if CODEC_MASTER
                command = deliver_slave(c_mix_out
#else
                command = deliver_master(c_mix_out
#endif
#if (XUA_SPDIF_TX_EN)
                   , c_spdif_out
#else
                   , null
#endif
#ifdef ADAT_TX
                   , c_adat_out
                   , adatSmuxMode
#endif
                   , divide, curSamFreq
#if defined (ADAT_RX) || defined (SPDIF_RX)
                   , c_dig_rx
#endif
#if (NUM_PDM_MICS > 0)
                   , c_pdm_in
#endif
                   );

#ifndef NO_USB
                if(command == SET_SAMPLE_FREQ)
                {
                    curSamFreq = inuint(c_mix_out) * AUD_TO_USB_RATIO;
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

#if (XUA_DFU_EN == 1)
                /* Currently no more audio will happen after this point */
                if ((curSamFreq / AUD_TO_USB_RATIO) == AUDIO_STOP_FOR_DFU)
                {
                    outct(c_mix_out, XS1_CT_END);

                    outuint(c_mix_out, 0);

                    while (1)
                    {
#if (XUD_TILE != 0) && (AUDIO_IO_TILE == 0)
                       [[combine]]
                        par
                        {
                            DFUHandler(dfuInterface, null);
                            dummy_deliver(c_mix_out, command);
                        }
#else
                        dummy_deliver(c_mix_out, command);
#endif
                        curSamFreq = inuint(c_mix_out);

                        if (curSamFreq == AUDIO_START_FROM_DFU)
                        {
                            outct(c_mix_out, XS1_CT_END);
                            break;
                        }
                    }
                }
#endif

#endif /* NO_USB */

#if (XUA_SPDIF_TX_EN)
                /* Notify S/PDIF task of impending new freq... */
                outct(c_spdif_out, XS1_CT_END);
#endif

#if NUM_PDM_MICS > 0
                c_pdm_in <: 0;
#endif

#ifdef ADAT_TX
#ifdef ADAT_TX_USE_SHARED_BUFF
                /* Take out-standing handshake from ADAT core */
                inuint(c_adat_out);
#endif
                /* Notify ADAT Tx thread of impending new freq... */
                outct(c_adat_out, XS1_CT_END);
#endif
            }
        }
    }
}

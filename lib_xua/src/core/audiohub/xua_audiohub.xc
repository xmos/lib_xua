// Copyright 2011-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
/**
 * @file xua_audiohub.xc
 * @brief XMOS USB 2.0 Audio Reference Design.  Audio Functions.
 * @author Ross Owen, XMOS Semiconductor Ltd
 *
 * This thread handles I2S and forwards samples to the SPDIF Tx core.
 * Additionally this thread handles clocking and CODEC/DAC/ADC config.
 **/

#include <syscall.h>
#include <platform.h>
#include <xs1.h>
#include <xclib.h>
#include <xs1_su.h>
#include <string.h>
#include <xassert.h>


#include "xua.h"

#include "audiohw.h"
#include "audioports.h"
#include "mic_array_conf.h"
#if (XUA_SPDIF_TX_EN)
#include "spdif.h"
#endif
#if (XUA_ADAT_TX_EN)
#include "adat_tx.h"
#ifndef ADAT_TX_USE_SHARED_BUFF
#error Designed for ADAT tx shared buffer mode ONLY
#endif
#endif

#if (XUA_NUM_PDM_MICS > 0)
#include "xua_pdm_mic.h"
#endif

#if (AUD_TO_USB_RATIO > 1)
#include "src.h"
#endif

#include "xua_commands.h"
#include "xc_ptr.h"

#include "debug_print.h"

#define XUA_MAX(x,y) ((x)>(y) ? (x) : (y))

unsigned samplesOut[XUA_MAX(NUM_USB_CHAN_OUT, I2S_CHANS_DAC)];

/* Two buffers for ADC data to allow for DAC and ADC I2S ports being offset */
#define IN_CHAN_COUNT (I2S_CHANS_ADC + XUA_NUM_PDM_MICS + (8*XUA_ADAT_RX_EN) + (2*XUA_SPDIF_RX_EN))

unsigned samplesIn[2][XUA_MAX(NUM_USB_CHAN_IN, IN_CHAN_COUNT)];

#if (XUA_ADAT_TX_EN)
extern buffered out port:32 p_adat_tx;
#endif

#if (XUA_ADAT_TX_EN)
extern clock    clk_mst_spd;
#endif

#if CODEC_MASTER
void InitPorts_slave
#else
void InitPorts_master
#endif
(buffered _XUA_CLK_DIR port:32 p_lrclk, buffered _XUA_CLK_DIR port:32 p_bclk, buffered out port:32 (&?p_i2s_dac)[I2S_WIRES_DAC],
    buffered in port:32  (&?p_i2s_adc)[I2S_WIRES_ADC]);


unsigned dsdMode = DSD_MODE_OFF;

#if (DSD_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT > 0)
#include "audiohub_dsd.h"
#endif

#if (XUA_ADAT_TX_EN)
#include "audiohub_adat.h"
#endif
#include "xua_audiohub_st.h"

static inline int HandleSampleClock(int frameCount, buffered _XUA_CLK_DIR port:32 p_lrclk)
{
#if CODEC_MASTER
    unsigned syncError = 0;
    unsigned lrval = 0;
    const unsigned lrval_mask = (0xffffffff << (32 - XUA_I2S_N_BITS));

    if(XUA_I2S_N_BITS != 32)
    {
        asm volatile("in %0, res[%1]":"=r"(lrval):"r"(p_lrclk):"memory");
        set_port_shift_count(p_lrclk, XUA_I2S_N_BITS);
    }
    else
    {
        p_lrclk :> lrval;
    }

    if(XUA_PCM_FORMAT == XUA_PCM_FORMAT_TDM)
    {
        /* Only check for the rising edge of frame sync being in the right place because falling edge timing not specified */
        if (frameCount == 1)
        {
            lrval &= 0xc0000000;                 // Mask off last two (MSB) frame clock bits which are the most recently sampled
            syncError += (lrval != 0x80000000);  // We need MSB = 1 and MSB-1 = 0 to signify rising edge
        }
        else
        {
            /* We do not check this part of the frame because TDM frame sync falling egde timing
             * is not defined. We only care about rising edge which is checked in first half of frame */
        }
    }
    else
    {
        if(XUA_I2S_N_BITS == 32)
        {
            if(frameCount == 0)
                syncError = (lrval != 0x80000000);
            else
                syncError = (lrval != 0x7FFFFFFF);
        }
        else
        {
            if(frameCount == 0)
                syncError = ((lrval & lrval_mask) != 0x80000000);
            else
                syncError = ((lrval | (~lrval_mask)) != 0x7FFFFFFF);
        }
    }

    return syncError;

#else
    unsigned clkVal;
    if(XUA_PCM_FORMAT == XUA_PCM_FORMAT_TDM)
    {
        if(frameCount == (I2S_CHANS_PER_FRAME-1))
            clkVal = 0x80000000;
        else
            clkVal = 0x00000000;
    }
    else
    {
        if(frameCount == 0)
            clkVal = 0x80000000;
        else
            clkVal = 0x7fffffff;
    }

    if(XUA_I2S_N_BITS == 32)
        p_lrclk <: clkVal;
    else
        partout(p_lrclk, XUA_I2S_N_BITS, clkVal >> (32 - XUA_I2S_N_BITS));

    return 0;
#endif

}

#pragma unsafe arrays
unsigned static AudioHub_MainLoop(chanend ?c_aud, chanend ?c_spd_out
#if (XUA_ADAT_TX_EN)
    , chanend c_adat_out
    , unsigned adatSmuxMode
#endif
    , unsigned divide, unsigned curSamFreq
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
    , chanend c_dig_rx
#endif
#if (XUA_NUM_PDM_MICS > 0)
    , chanend c_pdm_pcm
#endif
    , buffered _XUA_CLK_DIR port:32 ?p_lrclk,
    buffered _XUA_CLK_DIR port:32 ?p_bclk,
    buffered out port:32 (&?p_i2s_dac)[I2S_WIRES_DAC],
    buffered in port:32  (&?p_i2s_adc)[I2S_WIRES_ADC]
)
{
    /* Since DAC and ADC buffered ports off by one sample we buffer previous ADC frame */
    unsigned readBuffNo = 0;
    unsigned index;

#if (DSD_CHANS_DAC != 0)
    unsigned dsdMarker = DSD_MARKER_2;    /* This alternates between DSD_MARKER_1 and DSD_MARKER_2 */
    int dsdCount = 0;
    int everyOther = 1;
    unsigned dsdSample_l = 0x96960000;
    unsigned dsdSample_r = 0x96960000;
#endif
    unsigned underflowWord = 0;

#if (XUA_ADAT_TX_EN)
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
#if (XUA_NUM_PDM_MICS > 0)
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

    UserBufferManagementInit(curSamFreq);

    /* Get initial samples for first I2S output */
    unsigned command = DoSampleTransfer(c_aud, readBuffNo, underflowWord);

    // Reinitialise user state before entering the main loop
    UserBufferManagementInit(curSamFreq);

#if XUA_NUM_PDM_MICS > 0
    /* Receive an initial frame and wait one sample period to ensure mic_array is ready to produce so
       we don't break the audioloop timing by ma_frame_rx() blocking */
    unsafe {
        chanend_t c_m2a = (chanend_t)c_pdm_pcm;
        int32_t *mic_samps_base_addr = (int32_t*)&samplesIn[readBuffNo][PDM_MIC_INDEX];
        ma_frame_rx(mic_samps_base_addr, c_m2a, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME, MIC_ARRAY_CONFIG_MIC_COUNT);
        delay_ticks(XS1_TIMER_HZ / curSamFreq);
    }
#endif

#if (XUA_ADAT_TX_EN)
    unsafe{
    //TransferAdatTxSamples(c_adat_out, samplesOut, adatSmuxMode, 0);
    volatile unsigned * unsafe samplePtr = &samplesOut[ADAT_TX_INDEX];
    outuint(c_adat_out, (unsigned) samplePtr);
    }
#endif
    if(command != XUA_AUDCTL_NO_COMMAND)
    {
        return command;
    }

    /* Main Audio I/O loop */
    while (1)
    {
        unsigned syncError = 0;
        unsigned frameCount = 0;

        if ((I2S_CHANS_DAC > 0 || I2S_CHANS_ADC > 0))
        {
#if CODEC_MASTER
            InitPorts_slave(p_lrclk, p_bclk, p_i2s_dac, p_i2s_adc);
#else
            InitPorts_master(p_lrclk, p_bclk, p_i2s_dac, p_i2s_adc);
#endif
        }

        /* Note we always expect syncError to be 0 when we are master */
        while(!syncError)
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
                    asm volatile("in %0, res[%1]" : "=r"(sample)  : "r"(p_i2s_adc[index]));

                    sample = bitrev(sample);
                    if(XUA_I2S_N_BITS != 32)
                    {
                        set_port_shift_count(p_i2s_adc[index], XUA_I2S_N_BITS);
                        sample <<= (32 - XUA_I2S_N_BITS);
                    }
                    index++;

                    int chanIndex = ((frameCount-2) & (I2S_CHANS_PER_FRAME-1)) + i; // channels 0, 2, 4.. on each line.

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

#if (I2S_CHANS_ADC != 0 || I2S_CHANS_DAC != 0)
                syncError += HandleSampleClock(frameCount, p_lrclk);
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
                    if(XUA_I2S_N_BITS == 32)
                        p_i2s_dac[index++] <: bitrev(samplesOut[frameCount +i]);
                    else
                        partout(p_i2s_dac[index++], XUA_I2S_N_BITS, bitrev(samplesOut[frameCount +i]));
                }
#endif // (I2S_CHANS_DAC != 0)

            if(frameCount == 0)
            {
#if (XUA_ADAT_TX_EN)
                TransferAdatTxSamples(c_adat_out, samplesOut, adatSmuxMode, 1);
#endif

#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
                /* Sync with clockgen */
                inuint(c_dig_rx);

                /* Note, digi-data we just store in samplesIn[readBuffNo] - we only double buffer the I2S input data */
#endif
#if (XUA_SPDIF_RX_EN)
                asm("ldw %0, dp[g_digData]"  :"=r"(samplesIn[readBuffNo][SPDIF_RX_INDEX + 0]));
                asm("ldw %0, dp[g_digData+4]":"=r"(samplesIn[readBuffNo][SPDIF_RX_INDEX + 1]));
#endif
#if (XUA_ADAT_RX_EN)
                asm("ldw %0, dp[g_digData+8]" :"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX]));
                asm("ldw %0, dp[g_digData+12]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 1]));
                asm("ldw %0, dp[g_digData+16]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 2]));
                asm("ldw %0, dp[g_digData+20]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 3]));
                asm("ldw %0, dp[g_digData+24]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 4]));
                asm("ldw %0, dp[g_digData+28]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 5]));
                asm("ldw %0, dp[g_digData+32]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 6]));
                asm("ldw %0, dp[g_digData+36]":"=r"(samplesIn[readBuffNo][ADAT_RX_INDEX + 7]));
#endif

#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
                /* Request digital data (with prefill) */
                outuint(c_dig_rx, 0);
#endif
#if (XUA_SPDIF_TX_EN) && (NUM_USB_CHAN_OUT > 0)
                outuint(c_spd_out, samplesOut[SPDIF_TX_INDEX]);  /* Forward samples to S/PDIF Tx thread */
                outuint(c_spd_out, samplesOut[SPDIF_TX_INDEX + 1]);
#endif

#if (XUA_NUM_PDM_MICS > 0)
                if ((AUD_TO_MICS_RATIO - 1) == audioToMicsRatioCounter)
                unsafe {
                    chanend_t c_m2a = (chanend_t)c_pdm_pcm;
                    int32_t *mic_samps_base_addr = (int32_t*)&samplesIn[readBuffNo][PDM_MIC_INDEX];
                    ma_frame_rx(mic_samps_base_addr, c_m2a, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME, MIC_ARRAY_CONFIG_MIC_COUNT);
                    user_pdm_process(mic_samps_base_addr);
                    audioToMicsRatioCounter = 0;
                }
                else
                {
                    ++audioToMicsRatioCounter;
                }
#endif
            }

           frameCount++;

#if (I2S_CHANS_ADC != 0)
                index = 0;
                /* Channels 0, 2, 4.. on each line */
#pragma loop unroll
                for(int i = 0; i < I2S_CHANS_ADC; i += I2S_CHANS_PER_FRAME)
                {
                    /* Manual IN instruction since compiler generates an extra setc per IN (bug #15256) */
                    unsigned sample;
                    asm volatile("in %0, res[%1]" : "=r"(sample)  : "r"(p_i2s_adc[index]));
                    sample = bitrev(sample);
                    if(XUA_I2S_N_BITS != 32)
                    {
                        set_port_shift_count(p_i2s_adc[index], XUA_I2S_N_BITS);
                        sample <<= (32 - XUA_I2S_N_BITS);
                    }
                    index++;

                    int chanIndex = ((frameCount-2)&(I2S_CHANS_PER_FRAME-1))+i; // channels 1, 3, 5.. on each line.
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
#endif /* I2S_CHANS_ADC != 0) */

#if (I2S_CHANS_ADC != 0 || I2S_CHANS_DAC != 0)
                syncError += HandleSampleClock(frameCount, p_lrclk);
#endif

                index = 0;
#if (I2S_CHANS_DAC != 0)
                /* Output "odd" channel to DAC (i.e. right) */
#pragma loop unroll
                for(int i = 0; i < I2S_CHANS_DAC; i+=I2S_CHANS_PER_FRAME)
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
                    if(XUA_I2S_N_BITS == 32)
                        p_i2s_dac[index++] <: bitrev(samplesOut[frameCount + i]);
                    else
                        partout(p_i2s_dac[index++], XUA_I2S_N_BITS, bitrev(samplesOut[frameCount + i]));
                }
#endif // (I2S_CHANS_DAC != 0)

            }  // !dsdMode


#if (DSD_CHANS_DAC != 0) && (NUM_USB_CHAN_OUT > 0)
            if(DoDsdDopCheck(dsdMode, dsdCount, curSamFreq, samplesOut, dsdMarker) == 0)
            {
#if (I2S_CHANS_ADC != 0) || (I2S_CHANS_DAC != 0)
                // Set clocks low
                p_lrclk <: 0;
                p_bclk <: 0;
#endif
                p_dsd_clk <: 0;
                return 0;
            }
#endif

#if (XUA_PCM_FORMAT == XUA_PCM_FORMAT_TDM)
            /* Increase frameCount by 2 since we have output two channels (per data line) */
            frameCount+=1;
            if(frameCount == I2S_CHANS_PER_FRAME)
#endif
            {
                if ((AUD_TO_USB_RATIO - 1) == audioToUsbRatioCounter)
                {
                    /* Do samples transfer */
                    /* The below looks a bit odd but forces the compiler to inline twice */
                    unsigned command;
                    if(readBuffNo)
                        command = DoSampleTransfer(c_aud, 1, underflowWord);
                    else
                        command = DoSampleTransfer(c_aud, 0, underflowWord);

                    if(command != XUA_AUDCTL_NO_COMMAND)
                    {
                        return command;
                    }

                    /* Reset audio to usb counter because we have now completed one USB transfer and flip the ADC buffer */
                    audioToUsbRatioCounter = 0;
                    readBuffNo = !readBuffNo;
                }
                else
                {
                    ++audioToUsbRatioCounter;
                }
                /* Reset the framecount because we have outputted all channels in the frame now */
                frameCount = 0;
            }
        }
    }
    return 0;
}

/* This helper function receives the command using the right transaction from Decouple when audiohub breaks */
static void receive_command(unsigned command,
                            chanend c_aud,
                            unsigned &curSamFreq,
                            unsigned &dsdMode,
                            unsigned &curSamRes_DAC,
                            unsigned &audioActive)
{
    debug_printf("receive_command: %d\n", command);
    if(command == XUA_AUDCTL_SET_SAMPLE_FREQ)
    {
        curSamFreq = inuint(c_aud) * AUD_TO_USB_RATIO;
        debug_printf("receive_command set sr: %d\n", curSamFreq);
    }
    else if(command == XUA_AUD_SET_AUDIO_START)
    {
        /* Off = 0
         * DOP = 1
         * Native = 2
         */
        dsdMode = inuint(c_aud);
        curSamRes_DAC = inuint(c_aud);
        audioActive = 1;
        debug_printf("aud stream start\n")
    }
    else if (command == XUA_AUD_SET_AUDIO_STOP)
    {
        debug_printf("aud stream stop\n");
        if(XUA_LOW_POWER_NON_STREAMING)
        {
            audioActive = 0;
        }
    }
    else
    {
        debug_printf("aud unhandled cmd  %u\n", command);
    }
    /* Not we do not ACK back here - it is done when we re-start audio */
}

#if XUA_DFU_EN
[[distributable]]
void DFUHandler(server interface i_dfu i, chanend ?c_user_cmd);
#endif


/* This function is a dummy version of the deliver thread that does not
   connect to the codec ports. It is used during DFU reset and during idle non-streaming mode, if enabled.
   Note there are two paths through depending on dfuMode.*/
[[combinable]]
static void dummy_deliver(chanend ?c_aud, unsigned sampFreq, unsigned dfuMode, unsigned &command)
{
    const int wait_ticks = XS1_TIMER_HZ / sampFreq;
    timer tmr;
    int tmr_trigger;
    tmr :> tmr_trigger;
    tmr_trigger += wait_ticks;

    if(!dfuMode)
    {
        StartSampleTransfer(c_aud, 0);
    }

/* This is a bit annoyting but we have to end in a while(1) select to be combinable */
#if ((NUM_USB_CHAN_OUT > 0) || (NUM_USB_CHAN_IN > 0))
#define COMPLETE_SAMPLE_TRANSFER(c, buf, cmd)   CompleteSampleTransferUsbChans(c, buf, cmd)
#else
#define COMPLETE_SAMPLE_TRANSFER(c, buf, cmd)   CheckForCmdNoUsbChans(c, buf, cmd)
#endif
    while (1)
    {
        /* Note, a select is used such that this task is combinable */
        select
        {
            case COMPLETE_SAMPLE_TRANSFER(c_aud, 0, command):   /* Check for command & transfer the samples & UBM */
                if(command)
                {
                    if(dfuMode)
                    {
                        /* Just consume the command, ignore it + keep on looping forever */
                        unsigned dummy1, dummy2, dummy3, dummy4;
                        receive_command(command, c_aud, dummy1, dummy2, dummy3, dummy4);
                        outct(c_aud, XS1_CT_END);
                    }
                    else
                    {
                        /* Process the command in the callee */
                        return;
                    }
                }
                /* Wait one sample period */
                tmr when timerafter(tmr_trigger) :> void;
                tmr_trigger += wait_ticks;

                /* Request more data/commands */
                if((NUM_USB_CHAN_OUT > 0) || (NUM_USB_CHAN_IN > 0))
                {
                    StartSampleTransfer(c_aud, 0);
                }
            break;
        }
    }
}


#if XUA_DFU_EN
/* External DFU handler task */
[[distributable]]
void DFUHandler(server interface i_dfu i, chanend ?c_user_cmd);


/* Helper function to see if a request for entry to DFU has been issued via a sample rate change.
   If so, enter DFU. Note this code will never return. The device will
   need to be reset which is part of the DFU sequence */
void check_and_enter_dfu(unsigned curSamFreq, chanend c_aud, server interface i_dfu ?dfuInterface)
{
    /* Currently no more audio will happen after this point */
    if ((curSamFreq / AUD_TO_USB_RATIO) == AUDIO_STOP_FOR_DFU)
    {
        /* Handshake back the SR change command that put us in DFU*/
        outct(c_aud, XS1_CT_END);

        /* Request more data/commands */
        if((NUM_USB_CHAN_OUT > 0) || (NUM_USB_CHAN_IN > 0))
        {
            StartSampleTransfer(c_aud, 0);          /* Send first token to fire ISR in decouple */
        }

        unsigned command = XUA_AUDCTL_NO_COMMAND;
        while (1)
        {
           [[combine]]
            par
            {
#if (XUD_TILE != 0) && (AUDIO_IO_TILE == 0)
                DFUHandler(dfuInterface, null);
#endif
                /* This never exits because we set DFU mode*/
                dummy_deliver(c_aud, 48000, 1, command);
            }
            /* Note, we shouldn't reach here. Audio, once stopped for DFU, cannot be resumed */
        }
    }
}
#endif /* XUA_DFU_EN */


void XUA_AudioHub(chanend ?c_aud, clock ?clk_audio_mclk, clock ?clk_audio_bclk,
    in port ?p_mclk_in,
    buffered _XUA_CLK_DIR port:32 ?p_lrclk,
    buffered _XUA_CLK_DIR port:32 ?p_bclk,
    buffered out port:32 (&?p_i2s_dac)[I2S_WIRES_DAC],
    buffered in port:32  (&?p_i2s_adc)[I2S_WIRES_ADC]
#if (XUA_SPDIF_TX_EN) //&& (SPDIF_TX_TILE != AUDIO_IO_TILE)
    , chanend c_spdif_out
#endif
#if (XUA_ADAT_RX_EN || XUA_SPDIF_RX_EN)
    , chanend c_dig_rx
#endif
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC || XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
    , chanend c_audio_rate_change
#endif
#if (XUD_TILE != 0) && (AUDIO_IO_TILE == 0) && (XUA_DFU_EN == 1)
    , server interface i_dfu ?dfuInterface
#endif
#if (XUA_NUM_PDM_MICS > 0)
    , chanend c_pdm_in
#endif
)
{
/* This is a bit annoying but we have a mixture of nullable interfaces and variadic function signatures based on defines */
#if !((XUD_TILE != 0) && (AUDIO_IO_TILE == 0) && (XUA_DFU_EN == 1))
#define dfuInterface null
#endif
#if (XUA_ADAT_TX_EN)
    chan c_adat_out;
    unsigned adatSmuxMode = 0;
    unsigned adatMultiple = 0;
#endif
    unsigned curSamFreq = DEFAULT_FREQ * AUD_TO_USB_RATIO;
    unsigned curSamRes_DAC = STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS; /* Default to something reasonable */
    unsigned curSamRes_ADC = STREAM_FORMAT_INPUT_1_RESOLUTION_BITS; /* Default to something reasonable - note, currently this never changes*/
    unsigned command = XUA_AUDCTL_NO_COMMAND;
    unsigned mClk;
    unsigned divide;
    /* Flag used to indicate whether both interfaces are set to Alt 0 or not for power saving option
       This is only used when XUA_LOW_POWER_NON_STREAMING is defined to non-zero */
    unsigned audioActive = XUA_LOW_POWER_NON_STREAMING ? 0 : 1;
    /* This flag is to ensure that the decouple<->audio channel protocol is observed at startup.
       We need this because we hold off the ACK back to decouple as late as possible so that the control path knows audio is fully ready */
    unsigned firstRun = 1;
    /* Mic array currently doesn't support reconfig. So for now we keep a flag to ensure we only the init transaction it once and only when running I2S */
    unsigned micArrayInitisalised = 0;

    while(1)
    {
        /* This is the "low power non-streaming" loop */
        while(audioActive == 0)
        {
            /* We only want to ACK after we have done our first transaction with decouple */
            if(firstRun == 0)
            {
                /* Handshake back previous command */
                if(XUA_USB_EN)
                {
                    outct(c_aud, XS1_CT_END);
                }
            }
            else
            {
                firstRun = 0;
            }
            /* Now run dummy loop with no IO. This is sufficient to poll for commands from decouple */
            dummy_deliver(c_aud, 1000, 0, command);  /* Run loop at 1kHz for min power and exit if command */
            receive_command(command, c_aud, curSamFreq, dsdMode, curSamRes_DAC, audioActive);
#if (XUA_DFU_EN == 1)
            check_and_enter_dfu(curSamFreq, c_aud, dfuInterface);
#endif /* (XUA_DFU_EN == 1) */
        } /* audioActive == 0 */

#if (DSD_CHANS_DAC > 0)
        /* Make sure the DSD ports are on and buffered - just in case they are not shared with I2S */
        EnableBufferedPort(p_dsd_clk, 32);
        for(int i = 0; i< DSD_CHANS_DAC; i++)
        {
            EnableBufferedPort(p_dsd_dac[i], 32);
        }
#endif

#if ((AUDIO_IO_TILE == XUD_TILE) || XUA_ADAT_TX_EN || XUA_SPDIF_TX_EN)
        xassert((!isnull(clk_audio_mclk) && !isnull(p_mclk_in)) && "Error: must provide non-null MCLK port and MCLK clock-block if digital Rx is enabled or AUDIO_IO_TILE==XUD_TILE");
        /* Clock master clock-block from master-clock port */
        configure_clock_src(clk_audio_mclk, p_mclk_in);
#if (XUA_ADAT_TX_EN)
        /* Set ADAT Tx port to be clock from master clock-block*/
        configure_out_port_no_ready(p_adat_tx, clk_audio_mclk, 0);
        set_clock_fall_delay(clk_audio_mclk, 7);
#endif
#endif

/* If the XUD tile is different from AUDIO tile, then we start a clkblk for counting clocks on the XUD tile and start it in main.
   If XUD is on the same tile as AUDIO then we just connect p_for_mclk_count to the  clk_audio_mclk in main, but
   we need to start it here after all of the connections have been made.
   Note. we do not need a clk_audio_mclk if no dig Tx and XUD and AUDIO are on different tiles because I2S is driven by BCLK clkblk
   directly from the MCLK port. */
#if ((AUDIO_IO_TILE == XUD_TILE) || XUA_ADAT_TX_EN || XUA_SPDIF_TX_EN)
        /* Start the master clock-block */
        start_clock(clk_audio_mclk);
#endif

        /* Perform required CODEC/ADC/DAC initialisation */
        AudioHwInit();

        /* Only break this loop if LP non streaming enabled and streams are both Alt 0 */
        while((XUA_LOW_POWER_NON_STREAMING == 0) || audioActive)
        {
            /* Calculate what master clock we should be using */
            if (((MCLK_441) % curSamFreq) == 0)
            {
                mClk = MCLK_441;
#if (XUA_ADAT_TX_EN)
                /* Calculate ADAT SMUX mode (1, 2, 4) */
                adatSmuxMode = curSamFreq / 44100;
                adatMultiple = mClk / 44100;
#endif
            }
            else if (((MCLK_48) % curSamFreq) == 0)
            {
                mClk = MCLK_48;
#if (XUA_ADAT_TX_EN)
                /* Calculate ADAT SMUX mode (1, 2, 4) */
                adatSmuxMode = curSamFreq / 48000;
                adatMultiple = mClk / 48000;
#endif
            }

            /* Calculate master clock to bit clock (or DSD clock) divide for current sample freq
             * e.g. 11.289600 / (176400 * 64)  = 1 */
            {
                unsigned numBits = XUA_I2S_N_BITS * I2S_CHANS_PER_FRAME;

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
                divide = mClk / (curSamFreq * numBits);

                // Do some checks
                xassert((divide > 0) && "Error: divider is 0, BCLK rate unachievable");

                unsigned remainder = mClk % ( curSamFreq * numBits);
                xassert((!remainder) && "Error: MCLK not divisible into BCLK by an integer number");

                /* Ignore special divide = 1 case */
                unsigned divider_is_odd = (divide & 0x1) && (divide != 1);
                xassert((!divider_is_odd) && "Error: divider is odd, clockblock cannot produce desired BCLK");

           }

#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
#if (DSD_CHANS_DAC > 0)
            if(dsdMode)
            {
                /* Configure audio ports */
                ConfigAudioPortsWrapper(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
                    p_dsd_dac,
                    DSD_CHANS_DAC,
#endif // (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
#if (I2S_CHANS_ADC != 0)
                    p_i2s_adc,
                    I2S_WIRES_ADC,
#endif // (I2S_CHANS_ADC != 0)
                    null,
                    p_dsd_clk,
                    p_mclk_in, clk_audio_bclk, divide, curSamFreq);
            }
            else
#endif // (DSD_CHANS_DAC > 0)
            {
                ConfigAudioPortsWrapper(
#if (I2S_CHANS_DAC != 0)
                    p_i2s_dac,
                    I2S_WIRES_DAC,
#endif // (I2S_CHANS_DAC != 0)
#if (I2S_CHANS_ADC != 0)
                    p_i2s_adc,
                    I2S_WIRES_ADC,
#endif // (I2S_CHANS_ADC != 0)
                    p_lrclk,
                    p_bclk,
                    p_mclk_in, clk_audio_bclk, divide, curSamFreq);
            }
#endif // (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)

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

                /* User should mute audio hardware */
                AudioHwConfig_Mute();

                /* User code should configure audio harware for SampleFreq/MClk etc */
                AudioHwConfig(curFreq, mClk, dsdMode, curSamRes_DAC, curSamRes_ADC);
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC || XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
                /* Notify clockgen of new mCLk */
                c_audio_rate_change <: mClk;
                c_audio_rate_change <: curFreq;

                /* Wait for ACK back from clockgen or ep_buffer to signal clocks all good */
                c_audio_rate_change :> int _;
#endif

                /* User should unmute audio hardware */
                AudioHwConfig_UnMute();
            }

            if(micArrayInitisalised == 0)
            {
#if (XUA_NUM_PDM_MICS > 0)
            /* Send decimation factor to PDM task(s) */
            user_pdm_init();
            c_pdm_in <: curSamFreq / AUD_TO_MICS_RATIO;
#endif
                micArrayInitisalised = 1;
            }

            if(firstRun == 0)
            {
                /* TODO wait for good mclk instead of delay */
                /* No delay for DFU modes */
                if (command)
                {
#if 0
                    /* User should ensure MCLK is stable in AudioHwConfig */
                    if(retVal1 == XUA_AUDCTL_SET_SAMPLE_FREQ)
                    {
                        timer t;
                        unsigned time;
                        t :> time;
                        t when timerafter(time+AUDIO_PLL_LOCK_DELAY) :> void;
                    }
#endif
                    /* Handshake back from previous loop's command*/
                    if(XUA_USB_EN)
                    {
                        outct(c_aud, XS1_CT_END);
                    }
                }
            }
            firstRun = 0;

            par
            {

#if (XUA_ADAT_TX_EN)
                {
                    set_thread_fast_mode_on();
                    adat_tx_port(c_adat_out, p_adat_tx);
                }
#endif
                {
#if (XUA_SPDIF_TX_EN)
                    /* Communicate master clock and sample freq to S/PDIF thread */
                    outct(c_spdif_out, XS1_CT_END);
                    outuint(c_spdif_out, curSamFreq);
                    outuint(c_spdif_out, mClk);
#endif

#if (XUA_ADAT_TX_EN)
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
                    command = AudioHub_MainLoop(c_aud
#if (XUA_SPDIF_TX_EN)
                       , c_spdif_out
#else
                       , null
#endif
#if (XUA_ADAT_TX_EN)
                       , c_adat_out
                       , adatSmuxMode
#endif
                       , divide, curSamFreq
#if (XUA_ADAT_RX_EN || XUA_SPDIF_RX_EN)
                       , c_dig_rx
#endif
#if (XUA_NUM_PDM_MICS > 0)
                       , c_pdm_in
#endif
                      , p_lrclk, p_bclk, p_i2s_dac, p_i2s_adc);

#if (XUA_USB_EN)
                    /* Now perform any additional inputs and update state accordingly */
                    receive_command(command, c_aud, curSamFreq, dsdMode, curSamRes_DAC, audioActive);
#if (XUA_DFU_EN == 1)
                    check_and_enter_dfu(curSamFreq, c_aud, dfuInterface);

#endif /* (XUA_DFU_EN == 1) */
#endif /* XUA_USB_EN */

#if XUA_NUM_PDM_MICS > 0
                    // TODO - this willbe an exit command when supported in mic_array
                    // c_pdm_in <: 0;
#endif

#if (XUA_ADAT_TX_EN)
#ifdef ADAT_TX_USE_SHARED_BUFF
                    /* Take out-standing handshake from ADAT core */
                    inuint(c_adat_out);
#endif
                    /* Notify ADAT Tx thread of impending new freq... */
                    outct(c_adat_out, XS1_CT_END);
#endif /* XUA_ADAT_TX_EN) */
                } /* AudioHub_MainLoop and command handler */
            } /* par */
        } /* while((!XUA_LOW_POWER_NON_STREAMING) || audioActive) */


        /* The following code can only be reached if XUA_LOW_POWER_NON_STREAMING is enabled and all streams stopped */

        /* First shutdown and reset ports before we may shutdown any clocks */
#if (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)
#if (DSD_CHANS_DAC > 0)
        if(dsdMode)
        {
            /* Configure audio ports */
            DeConfigAudioPorts(
#if (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
                p_dsd_dac,
                DSD_CHANS_DAC,
#endif // (I2S_CHANS_DAC != 0) || (DSD_CHANS_DAC != 0)
#if (I2S_CHANS_ADC != 0)
                p_i2s_adc,
                I2S_WIRES_ADC,
#endif // (I2S_CHANS_ADC != 0)
                null,
                p_dsd_clk,
                p_mclk_in,
                clk_audio_bclk);
        }
        else
#endif // (DSD_CHANS_DAC > 0)
        {
            DeConfigAudioPorts(
#if (I2S_CHANS_DAC != 0)
                p_i2s_dac,
                I2S_WIRES_DAC,
#endif // (I2S_CHANS_DAC != 0)
#if (I2S_CHANS_ADC != 0)
                p_i2s_adc,
                I2S_WIRES_ADC,
#endif // (I2S_CHANS_ADC != 0)
                p_lrclk,
                p_bclk,
                p_mclk_in,
                clk_audio_bclk);
        }
#endif // (I2S_CHANS_DAC != 0) || (I2S_CHANS_ADC != 0)

        /* Call user functions for core power down (eg. MCLK disable) and system component power down */
        AudioHwShutdown();

    } /* while(1)*/
}

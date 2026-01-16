// Copyright 2011-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xua_commands.h"

static inline void StartSampleTransfer(chanend ?c_aud, const unsigned underflowWord)
{
    if(XUA_USB_EN)
    {
        outuint(c_aud, underflowWord);
    }
}


#pragma unsafe arrays
#pragma select handler
static inline void CompleteSampleTransferUsbChans(chanend ?c_aud, const int readBuffNo, unsigned &command)
{
    /* Check for sample freq change (or other command) or new samples from mixer*/
    if(testct(c_aud))
    {
        command = inct(c_aud);
#ifndef CODEC_MASTER
        if(dsdMode == DSD_MODE_OFF)
        {
#if (I2S_CHANS_ADC != 0 || I2S_CHANS_DAC != 0)
            /* Set clocks low */
            p_lrclk <: 0;
            p_bclk <: 0;
#endif
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
            return;
    }
    else
    {
#if NUM_USB_CHAN_OUT > 0
#pragma loop unroll
        for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
        {
            int tmp = inuint(c_aud);
            samplesOut[i] = tmp;
        }
#else
        inuint(c_aud);
#endif
        /* Run user code */
        UserBufferManagement(samplesOut, samplesIn[readBuffNo]);

#if NUM_USB_CHAN_IN > 0
#pragma loop unroll
        for(int i = 0; i < NUM_USB_CHAN_IN; i++)
        {
            outuint(c_aud, samplesIn[readBuffNo][i]);
        }
#endif
    }

    command = XUA_AUDCTL_NO_COMMAND;
}


#pragma select handler
static inline void CheckForCmdNoUsbChans(chanend ?c_aud, const int readBuffNo, unsigned &command)
{
    /* Poll for CT coming in from USB */ 
    /* In this case USB is still enabled, even though we have no audio channels to/from
     * host. Check for cmd - only expecting STOP_AUDIO_FOR_DFU. */
    unsigned char cmd;
    select
    {
        case inct_byref(c_aud, cmd):
            command = (unsigned)cmd;
            break;
        default:
            command = XUA_AUDCTL_NO_COMMAND;
            break; 
    }

    /* Run user code */
    UserBufferManagement(samplesOut, samplesIn[readBuffNo]);
}



static inline unsigned DoSampleTransfer(chanend ?c_aud, const int readBuffNo, const unsigned underflowWord)
{
    if(XUA_USB_EN)
    {
        unsigned command;
        if((NUM_USB_CHAN_OUT > 0) || (NUM_USB_CHAN_IN > 0))
        {
            StartSampleTransfer(c_aud, underflowWord);                  /* Send first token to fire ISR in decouple */
            CompleteSampleTransferUsbChans(c_aud, readBuffNo, command); /* Check for command & transfer the samples & UBM */
        }
        else
        {
            CheckForCmdNoUsbChans(c_aud, readBuffNo, command); /* Check for command & UBM */
        }

        return command;
    }
    else
    {
        /* I2S without USB - just run user code */
        UserBufferManagement(samplesOut, samplesIn[readBuffNo]);

        return XUA_AUDCTL_NO_COMMAND;
    }
}

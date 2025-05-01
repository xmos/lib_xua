// Copyright 2011-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xua_commands.h"

static inline void StartSampleTransfer(chanend ?c_out, const unsigned underflowWord)
{
    if(XUA_USB_EN && ((NUM_USB_CHAN_OUT > 0) || (NUM_USB_CHAN_IN > 0)))
    {
        outuint(c_out, underflowWord);
    }
}


#pragma select handler
void testct_byref(chanend c, int &returnVal)
{
    returnVal = 0;
    if(testct(c))
        returnVal = 1;
}


#pragma unsafe arrays
#pragma select handler
static inline void CompleteSampleTransferUsbChans(chanend ?c_out, const int readBuffNo, unsigned &command)
{
    /* Check for sample freq change (or other command) or new samples from mixer*/
    if(testct(c_out))
    {
        command = inct(c_out);
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
            int tmp = inuint(c_out);
            samplesOut[i] = tmp;
        }
#else
        inuint(c_out);
#endif
        /* Run user code */
        UserBufferManagement(samplesOut, samplesIn[readBuffNo]);

#if NUM_USB_CHAN_IN > 0
#pragma loop unroll
        for(int i = 0; i < NUM_USB_CHAN_IN; i++)
        {
            outuint(c_out, samplesIn[readBuffNo][i]);
        }
#endif
    }

    command = XUA_AUDCTL_NO_COMMAND;
}

static inline void CompleteSampleTransferNoUsbChans(chanend ?c_out, const int readBuffNo, unsigned &command)
{
    if(XUA_USB_EN)
    {
        /* In this case USB is still enabled, even though we have no audio channels to/from
         * host. Check for cmd - only expecting STOP_AUDIO_FOR_DFU. The select above cannot be
         * used since EP0 is not expecting to be polled */
        unsigned char command = XUA_AUDCTL_NO_COMMAND;
        select
        {
            case inct_byref(c_out, command):
                break;

            default:
                break;
        }

        if(command)
            return;
    }
    /* Run user code */
    UserBufferManagement(samplesOut, samplesIn[readBuffNo]);

    command = XUA_AUDCTL_NO_COMMAND;
}



static inline unsigned DoSampleTransfer(chanend ?c_out, const int readBuffNo, const unsigned underflowWord)
{
    StartSampleTransfer(c_out, underflowWord);          /* Send first token to fire ISR in decouple */
    unsigned command;
    if(XUA_USB_EN && ((NUM_USB_CHAN_OUT > 0) || (NUM_USB_CHAN_IN > 0)))
    {
        CompleteSampleTransferUsbChans(c_out, readBuffNo, command);   /* Check for command & transfer the samples & UBM */
    }
    else
    {
        CompleteSampleTransferNoUsbChans(c_out, readBuffNo, command); /* Check for command & UBM */
    }
    return command;
}

// Copyright 2011-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma unsafe arrays
static inline unsigned DoSampleTransfer(chanend ?c_out, const int readBuffNo, const unsigned underflowWord)
{
    if(XUA_USB_EN)
    {
        outuint(c_out, underflowWord);

        /* Check for sample freq change (or other command) or new samples from mixer*/
        if(testct(c_out))
        {
            unsigned command = inct(c_out);
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
    }
    else
        UserBufferManagement(samplesOut, samplesIn[readBuffNo]);

    return 0;
}


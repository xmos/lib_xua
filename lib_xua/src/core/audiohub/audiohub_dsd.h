// Copyright 2018-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#if (DSD_CHANS_DAC != 0)
extern buffered out port:32 p_dsd_dac[DSD_CHANS_DAC];
extern buffered out port:32 p_dsd_clk;
#endif

/* I2S Data I/O*/
#if (I2S_CHANS_DAC != 0)
extern buffered out port:32 p_i2s_dac[I2S_WIRES_DAC];
#endif

#if (I2S_CHANS_ADC != 0)
extern buffered in port:32  p_i2s_adc[I2S_WIRES_ADC];
#endif

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
   to restart in I2S/PCM mode. */
static inline int DoDsdDopCheck(unsigned &dsdMode, int &dsdCount, unsigned curSamFreq, unsigned samplesOut[], unsigned &dsdMarker)
{
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
                return 0;
            }
        }
    }
    return 1;
}



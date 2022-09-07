// Copyright 2018-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

unsigned adatCounter = 0;
unsigned adatSamples[8];

#pragma unsafe arrays
static inline void TransferAdatTxSamples(chanend c_adat_out, const unsigned samplesFromHost[], int smux, int handshake)
{

    /* Do some re-arranging for SMUX.. */
    unsafe
    {
        unsigned * unsafe samplesFromHostAdat = (unsigned * unsafe) &samplesFromHost[ADAT_TX_INDEX];

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
            volatile unsigned * unsafe samplePtr = (unsigned * unsafe) &adatSamples;
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

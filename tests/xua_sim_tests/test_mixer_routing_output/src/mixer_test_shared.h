// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef TEST_SEED
#error TEST_SEED must be defined!
#endif

/* A limitation of the design is that the number of routable output destinations cannot be larger than NUM_USB_CHAN_OUT.
 * This is due to the transfer samples from Mixer to AudioHub tasks being in blocks of NUM_USB_CHAN_OUT.
 * This is not normally an issue - since every physical output interface channel on the device is normally derived from a
 * USB channel from the host, but it certainly is a restriction.
 */
#define CHANNEL_MAP_AUD_SIZE NUM_USB_CHAN_OUT

#define CHANNEL_MAP_USB_SIZE NUM_USB_CHAN_IN

/* Number of channel sources, the channel ordering is as follows
 * i.e.
 * [0:NUM_USB_CHAN_OUT-1] : Channels from USB Host
 * [NUM_USB_CHAN_OUT:NUM_USB_CHAN_IN-1] : Channels from Audio Interfaces
 * [NUM_USB_CHAN_N:MAX_MIX_COUNT-1] : Channels from Mixers
 * [MAX_MIX_COUNT]: "Off" (Essentially samples always 0)
 */
/* Note, One larger for an "off" channel for mixer sources" */
#define SOURCE_COUNT (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT + 1)

#define SET_EXPECTED (9)
#define TRIGGER      (7)

// Test sample format:
// byte[0]: Sample counter
// byte[1]: Channel
// byte[3]: Source (HOST:1/AUD IF:0)
#define SRC_HOST     (2)
#define SRC_AUDIF    (1)
#define SRC_OFF      (0) // Important that this is 0 since mixer will generate 0 samples for 'off'

#define GET_COUNT(x) ((x>>8) & 0xff)
#define GET_CHANNEL(x) ((x >> 16) & 0xff)
#define GET_SOURCE(x) ((x >> 24) & 0xff)

#define SET_COUNT(x, y) y = y & 0xff; x = x | (y<<8);
#define SET_CHANNEL(x, y) y = y & 0xff; x = x | (y<<16);
#define SET_SOURCE(x, y) x = x | (y<<24);

void exit(int);

#pragma select handler
static inline void testct_byref(chanend c, unsigned &isCt)
{
    isCt = testct(c);
}

void SendTrigger(chanend c_stim_ah, int count)
{
    for(int i = 0; i < count; i++)
        outuint(c_stim_ah, TRIGGER);
}

uint32_t CreateSample(uint32_t modelMixerOutput[], int src)
{
    uint32_t sample = 0;

    if(src == (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT))
    {
        SET_SOURCE(sample, SRC_OFF);
    }
    else if(src >= (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN))
    {
        src -= (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN);
        sample = modelMixerOutput[src];
    }
    else if (src >= NUM_USB_CHAN_IN)
    {
        SET_SOURCE(sample, SRC_AUDIF);
        src -= NUM_USB_CHAN_OUT;
        SET_CHANNEL(sample, src);
    }
    else
    {
        SET_SOURCE(sample, SRC_HOST);
        SET_CHANNEL(sample, src);
    }

    return sample;
}

void PrintSourceString(unsigned source)
{
    debug_printf(" ");
    if(source < NUM_USB_CHAN_OUT)
    {
        debug_printf("(DEVICE IN - HOST%d)", source);
    }
    else if(source < (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN))
    {
        debug_printf("(DEVICE IN - AudioIF %d)", source - NUM_USB_CHAN_OUT);
    }
    else if(source < (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT))
    {
        debug_printf("(MIX %d)", source - NUM_USB_CHAN_OUT - NUM_USB_CHAN_IN);
    }
    else
        debug_printf("(off)");
    debug_printf(" ");
}

void PrintDestString(unsigned map, unsigned dest)
{
    switch(map)
    {
        case SET_SAMPLES_TO_DEVICE_MAP:
            debug_printf("(DEVICE OUT - AudioIF)");
            break;
        case SET_SAMPLES_TO_HOST_MAP:
            debug_printf("(DEVICE OUT - HOST)");
            break;
    }
}

void PrintSample(unsigned sample)
{
    debug_printf("SOURCE: ");
    if(GET_SOURCE(sample) == SRC_HOST)
        debug_printf("HOST    ");
    else if(GET_SOURCE(sample) == SRC_AUDIF)
        debug_printf("AUDIF   ");
    else if(GET_SOURCE(sample) == SRC_OFF)
        debug_printf("OFF     ");
    else
        debug_printf("UNKNOWN ");

    debug_printf("CHANNEL: %d", GET_CHANNEL(sample));
}

/* Required by lib_xua */
void AudioHwInit()
{
    return;
}

/* Required by lib_xua */
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    return;
}

/* From xua_ep0_uacreqs.xc */
void UpdateMixerOutputRouting(chanend c_mix_ctl, unsigned map, unsigned dst, unsigned src);
void UpdateMixMap(chanend c_mix_ctl, int mix, int input, int src);
void UpdateMixerWeight(chanend c_mix_ctl, int mix, int index, unsigned val);

void CheckBlock(unsigned samplesOut[], uint32_t expectedOut[], size_t len)
{
    int fail = 0;;
    for(int j = 0; j < len; j++)
    {
        debug_printf("%d: Expected: ", j);
        PrintSample(expectedOut[j]);
        debug_printf("\n");
        if(expectedOut[j] != samplesOut[j])
        {
            printstr("ERROR: Actual:   ");
            PrintSample(samplesOut[j]);
            debug_printf(" (%x)", samplesOut[j]);
            printstr("\n");
            fail = 1;
        }
        //assert(expectedOut[j] == samplesOut[j]);
    }
    assert(!fail);
}

/* Sending expected also causes fake_audiohub and fake_decouple to run sample checks */
void SendExpected(chanend c_stim_ah, chanend c_stim_de, uint32_t modelOut[], uint32_t modelIn[])
{
    /* Send expected to AudioHub */
    outuint(c_stim_ah, SET_EXPECTED);

    for(int i = 0; i < CHANNEL_MAP_AUD_SIZE; i++)
    {
        outuint(c_stim_ah, modelOut[i]);
    }

    /* Wait for handshake back and move on to next test */
    inuint(c_stim_ah);

    /* Send expected to Decouple */
    outuint(c_stim_de, SET_EXPECTED);
    for(int i = 0; i < NUM_USB_CHAN_IN; i++)
    {
        outuint(c_stim_de, modelIn[i]);
    }

    /* Wait for handshake back and move on to next test */
    inuint(c_stim_de);
}




/* From xua_audiohub.xc */
extern unsigned samplesOut[NUM_USB_CHAN_OUT];
extern unsigned samplesIn[2][NUM_USB_CHAN_IN];
#include "xua_audiohub_st.h"

int Fake_XUA_AudioHub(chanend c_mix_aud, chanend c_stim)
{
    int readBuffNo = 0;
    unsigned underflowWord = 0;
    uint32_t expectedOut[NUM_USB_CHAN_OUT];
    unsigned ct = 0;
    unsigned cmd = 0;

    for(size_t i = 0; i < NUM_USB_CHAN_IN; i++)
    {
        /* Note, we only used readBufNo = 0 */
        unsigned sample = 0;
        SET_SOURCE(sample, SRC_AUDIF);
        SET_CHANNEL(sample, i);
        samplesIn[0][i] = sample;
    }

    while(!ct)
    {

        select
        {
            case testct_byref(c_stim, ct):

                if(!ct)
                {
                    cmd = inuint(c_stim);

                    switch(cmd)
                    {
                        case SET_EXPECTED:

                            for(int j = 0; j < NUM_USB_CHAN_OUT; j++)
                            {
                                expectedOut[j] = inuint(c_stim);
                            }

                            debug_printf("AudioHub:\n");
                            CheckBlock(samplesOut, expectedOut, NUM_USB_CHAN_OUT);
                            /* Handshake back */
                            outuint(c_stim, 0);
                            break;

                        case TRIGGER:
                            /* This will populate samplesOut and send out samplesIn[readBuffNo] */
                            unsigned command = DoSampleTransfer(c_mix_aud, readBuffNo, underflowWord);
                            break;

                        default:
                            printstr("ERROR: bad cmd in Fake_XUA_AudioHub: ");
                            printintln(cmd);
                            assert(0);
                            break;
                    }
                }
                break;
        }
    }

    outct(c_stim, XS1_CT_END);
    inct(c_stim);
    return 0;
}

int Fake_XUA_Buffer_Decouple(chanend c_dec_mix, chanend c_stim)
{
    uint32_t expectedSamplesIn[NUM_USB_CHAN_IN];
    unsigned samplesIn[NUM_USB_CHAN_IN];
    unsigned ct;
    unsigned underflowSample;

    while(!ct)
    {
        select
        {
            case inuint_byref(c_dec_mix, underflowSample):

                for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
                {
                    unsigned sample = 0;
                    SET_SOURCE(sample, SRC_HOST);
                    SET_CHANNEL(sample, i);
                    outuint(c_dec_mix, sample);
                }

                for(int i = 0; i < NUM_USB_CHAN_IN; i++)
                {
                    samplesIn[i] = inuint(c_dec_mix);
                }

                break;

            case testct_byref(c_stim, ct):

                if(!ct)
                {
                    inuint(c_stim); // TODO don't really need this

                    /* Get expected */
                    for(int j = 0; j < NUM_USB_CHAN_IN; j++)
                    {
                        expectedSamplesIn[j] = inuint(c_stim);
                    }

                    debug_printf("Decouple:\n");
                    CheckBlock(samplesIn, expectedSamplesIn, NUM_USB_CHAN_IN);

                    /* Handshake back */
                    outuint(c_stim, 0);
                }
                break;
        }
    }

    outct(c_stim, XS1_CT_END);
    inct(c_stim);
    return 0;
}



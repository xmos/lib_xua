// TODO 
// - use lib_random
// - use random seed from pytest

// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* Tests that routing of mixer outputs behaves as expected 
 *
 * "Outputs" from the device are to the USB host of one of the various audio interaces supported.

 * This test assumes/checks the default routing for the USB host & audio interfaces is as follows:
 *
 *   USB_FROM_HOST[0] -> AUD_INTERFACE_OUTPUT[0]
 *   USB_FROM_HOST[1] -> AUD_INTERFACE_OUTPUT[1]
 *   ...
 *   USB_TO_HOST[0]   <- AUD_INTERFACE_INPUT[0]
 *   USB_TO_HOST[1]   <- AUD_INTERFACE_INPUT[1]
 *   ...
 *
 * This test also assumes/checks that the default routing into each of the MIX_INPUTS inputs into
 * each of the M mixer units is as follows:
 *
 *   MIXER[0]:
 *    USB_FROM_HOST[0]  -> MIXER[0].INPUT[0] 
 *    USB_FROM_HOST[1]  -> MIXER[0].INPUT[1]
 *    ...
      USB_TO_HOST[0]    -> MIXER[0].INPUT[NUM_USB_CHAN_OUT]
      USB_TO_HOST[1]    -> MIXER[0].INPUT[NUM_USB_CHAN_OUT+1]
      ...

 *   MIXER[MAX_MIX_COUNT-1]:
 *    USB_FROM_HOST[0]  -> MIXER[MAX_MIX_COUNT-1].INPUT[0] 
 *    USB_FROM_HOST[1]  -> MIXER[MAX_MIX_COUNT-1].INPUT[1]
 *   ...
 *
 * (If the number of mixer inputs > NUM_USB_CHAN_OUT then see ordering in comment regarding
 * SOURCE_COUNT below)
 *
 * By default none of the MAX_MIX_COUNT output from the mixers are routed anywwhere, but this test ensures
 * that they can be.
 *
 * This test assumes that none of the mixer weights are changed.
 * This test does not test changing the inputs to the mixer.
*/
#include <stdint.h>
#include <stddef.h>
#include "platform.h"
#include "xua.h"
#define DEBUG_UNIT main
#include "debug_print.h"
#include "assert.h"
#include "random.h"

#ifndef TEST_ITERATIONS 
#define TEST_ITERATIONS (100)
#endif

#ifndef TEST_SEED
#error TEST_SEED must be defined!
#endif

void exit(int);

// Test sample format:
// byte[0]: Sample counter
// byte[1]: Channel
// byte[3]: Source (HOST:1/AUD IF:0)
#define SRC_HOST     (2)
#define SRC_AUDIF    (1)
#define SRC_OFF      (0) // Important that this is 0 since mixer will generate 0 samples for 'off'

#define GET_COUNT(x) (x & 0xff)
#define GET_CHANNEL(x) ((x >> 8) & 0xff)
#define GET_SOURCE(x) ((x >> 16) & 0xff)

#define SET_COUNT(x, y) y = y & 0xff; x = x | y;
#define SET_CHANNEL(x, y) y = y & 0xff; x = x | (y<<8);
#define SET_SOURCE(x, y) x = x | (y<<16);

/* A limitation of the design is that the number of routable output destinations cannot be larger than NUM_USB_CHAN_OUT.
 * This is due to the transfer samples from Mixer to AudioHub tasks being in blocks of NUM_USB_CHAN_OUT.
 * This is not normally an issue - since every physical output interface channel on the device is normally derived from a 
 * USB channel from the host, but it certainly is a restriction.
 */
#define CHANNEL_MAP_AUD_SIZE NUM_USB_CHAN_OUT

/* Number of channel sources, the channel ordering is as follows
 * i.e. 
 * [0:NUM_USB_CHAN_OUT-1] : Channels from USB Host
 * [NUM_USB_CHAN_OUT:NUM_USB_CHAN_IN-1] : Channels from Audio Interfaces
 * [NUM_USB_CHAN_N:MAX_MIX_COUNT-1] : Channels from Mixers
 * [MAX_MIX_COUNT]: "Off" (Essentially samples always 0)
 */
/* Note, One larger for an "off" channel for mixer sources" */
#define SOURCE_COUNT (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT + 1)

#pragma select handler
static inline void testct_byref(chanend c, unsigned &isCt)
{
    isCt = testct(c);
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

void UpdateModel(uint32_t modelOut[CHANNEL_MAP_AUD_SIZE], uint32_t modelMixerOut[MAX_MIX_COUNT], uint32_t modelIn[NUM_USB_CHAN_IN],
     int map, int dst, int src)
{
    unsigned sample = 0;
    if(src == (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT))
    {
        SET_SOURCE(sample, SRC_OFF);
    }
    else if(src >= (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN))
    {
        src -= (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN);
        sample = modelMixerOut[src];
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

    switch(map)
    {
        case SET_SAMPLES_TO_DEVICE_MAP:
            modelOut[dst] = sample;
            break;
        
        case SET_SAMPLES_TO_HOST_MAP:
            modelIn[dst] = sample;
            break;
        
        default:
            assert(0);
            break;
    }
}

/* This task configures the routing and maintains a model of the expected routing output 
 * it provides this to the Fake AudioHub and Fake Decouple tasks such that they can self check 
 */
void stim(chanend c_stim_ah, chanend c_stim_de, chanend c_mix_ctl)
{
    uint32_t modelOut[CHANNEL_MAP_AUD_SIZE];
    uint32_t modelIn[NUM_USB_CHAN_IN];
    uint32_t modelMixerOut[MAX_MIX_COUNT];
    uint32_t testCmd[] = {SET_SAMPLES_TO_HOST_MAP, SET_SAMPLES_TO_DEVICE_MAP};

    random_generator_t rg = random_create_generator_from_seed(TEST_SEED);
    //assert(NUM_USB_CHAN_OUT >= MIX_INPUTS);

    /* By default the mixer should output samples from USB host unmodified
     * See mixer.xc L780
     */
    for(size_t i = 0; i < MAX_MIX_COUNT; i++)
    {
        uint32_t sample = 0;
        SET_SOURCE(sample, SRC_HOST);
        SET_CHANNEL(sample, i);
        modelMixerOut[i] = sample;
    }

    /* Init modelOut for default routing */
    /* Default routing is USB[0] -> AUD_IF[0] etc */
    for(size_t i = 0; i < CHANNEL_MAP_AUD_SIZE; i++)
    {
        uint32_t sample = 0;
        SET_SOURCE(sample, SRC_HOST);
        SET_CHANNEL(sample, i);
        modelOut[i] = sample;
    }

    /* Init modelIn for default routing */
    /* Default routing is AUD_IF[0] -> USB[0] etc */
    for(size_t i = 0; i < NUM_USB_CHAN_IN; i++)
    {
        uint32_t sample = 0;
        SET_SOURCE(sample, SRC_AUDIF);
        SET_CHANNEL(sample, i);
        modelIn[i] = sample;
    }

    outuint(c_stim_ah, 0);

    /* Check default routing */
    /* Send expected to AudioHub */
    for(int i = 0; i < CHANNEL_MAP_AUD_SIZE; i++)
    {
        outuint(c_stim_ah, modelOut[i]);
    }

    /* Wait for handshake back and move on to next test */
    inuint(c_stim_ah);

    for(int testIter = 0; testIter < TEST_ITERATIONS; testIter++)
    {
        /* Make a random update to the routing - route a random source to a random destination */
        unsigned map = testCmd[random_get_random_number(rg) % (sizeof(testCmd)/sizeof(testCmd[0]))];
        unsigned dst = random_get_random_number(rg)  % CHANNEL_MAP_AUD_SIZE;
        unsigned src = random_get_random_number(rg) % NUM_USB_CHAN_OUT; 

        switch(map)
        {
            case SET_SAMPLES_TO_DEVICE_MAP:
                debug_printf("Mapping output to AudioIF: %d", dst);
                PrintDestString(map, dst);
                debug_printf(" from %d", src);
                PrintSourceString(src);
                debug_printf("\n");

                /* Update the mixer */
                UpdateMixerOutputRouting(c_mix_ctl, map, dst, src); 
                break;

            case SET_SAMPLES_TO_HOST_MAP:
                debug_printf("Mapping output to Host : %d", dst);
                PrintDestString(map, dst);
                debug_printf(" from %d", src);
                PrintSourceString(src);
                debug_printf("\n");
        
                /* Update the mixer */
                UpdateMixerOutputRouting(c_mix_ctl, map, dst, src); 
                break;

            default:
                printstrln("ERROR BAD CMD");
              break;
        }

        /* Update the model */
        UpdateModel(modelOut, modelMixerOut, modelIn, map, dst, src);

        /* Send expected to AudioHub */
        outuint(c_stim_ah, 0);
        for(int i = 0; i < CHANNEL_MAP_AUD_SIZE; i++)
        {  
            outuint(c_stim_ah, modelOut[i]);
        }

        /* Wait for handshake back and move on to next test */
        inuint(c_stim_ah);

        /* Send expected to Decouple */
        outuint(c_stim_de, 0);
        for(int i = 0; i < NUM_USB_CHAN_IN; i++)
        {  
            outuint(c_stim_de, modelIn[i]);
        }

        /* Wait for handshake back and move on to next test */
        inuint(c_stim_de);
    }

    timer t;
    unsigned time;
    t :> time;
    t when timerafter(time+10000) :> void;

    /* Send kill messages to Fake AudioHub & Fake Decouple */
    outct(c_stim_ah, XS1_CT_END);
    inct(c_stim_ah);
    
    outct(c_stim_de, XS1_CT_END);
    inct(c_stim_de);
    
    printstrln("PASS");
    exit(0);
}

void CheckBlock(unsigned samplesOut[], uint32_t expectedOut[], size_t len)
{
    for(int j = 0; j < len; j++)
    {
        debug_printf("%d: Expected: ", j);
        PrintSample(expectedOut[j]);
        debug_printf("\n");
        if(expectedOut[j] != samplesOut[j])
        {
            printstr("ERROR: Actual:   ");
            PrintSample(samplesOut[j]);
        }
        assert(expectedOut[j] == samplesOut[j]);
    }
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
        /* This will populate samplesOut and send out samplesIn[readBuffNo] */
        unsigned command = DoSampleTransfer(c_mix_aud, readBuffNo, underflowWord);

        select
        {
            case testct_byref(c_stim, ct):

                if(!ct)
                {
                    inuint(c_stim); // TODO don't really need this 

                    /* Get expected */
                    for(int j = 0; j < NUM_USB_CHAN_OUT; j++)
                    {
                        expectedOut[j] = inuint(c_stim);
                    } 
                
                    CheckBlock(samplesOut, expectedOut, NUM_USB_CHAN_OUT);

                    /* Handshake back */
                    outuint(c_stim, 0);
                }
                break;

            default:
                break;
        }
    }

    outct(c_stim, XS1_CT_END);
    inct(c_stim);
    return 0;
}

int Fake_XUA_Buffer_Decouple(chanend c_dec_mix, chanend c_stim)
{  
    unsigned tmp;
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
                    
                    CheckBlock(samplesIn, expectedSamplesIn, NUM_USB_CHAN_IN);

                    /* Handshake back */
                    outuint(c_stim, 0);
                }
                break;

            default:
                break;
        }
    }
    
    outct(c_stim, XS1_CT_END);
    inct(c_stim);
    return 0;
}

int main()
{
    chan c_dec_mix;
    chan c_mix_aud;
    chan c_mix_ctl;
    chan c_stim_ah;
    chan c_stim_de;

    par
    {
        Fake_XUA_Buffer_Decouple(c_dec_mix, c_stim_de);
        Fake_XUA_AudioHub(c_mix_aud, c_stim_ah);
        
        /* Mixer from lib_xua */
        mixer(c_dec_mix, c_mix_aud, c_mix_ctl);

        stim(c_stim_ah, c_stim_de, c_mix_ctl);
    }

    /* TODO to hit this we need to fully close down i.e. kill mixer */
    return 0;
}

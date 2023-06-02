// Copyright 2022-2023 XMOS LIMITED.
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
#include "debug_print.h"
#include "assert.h"
#include "random.h"

#ifndef TEST_ITERATIONS
#define TEST_ITERATIONS (100)
#endif

#include "./mixer_test_shared.h"

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
    uint32_t modelIn[CHANNEL_MAP_USB_SIZE];
    uint32_t modelMixerOut[MAX_MIX_COUNT];
    uint32_t testCmd[] = {SET_SAMPLES_TO_HOST_MAP, SET_SAMPLES_TO_DEVICE_MAP};

    random_generator_t rg = random_create_generator_from_seed(TEST_SEED);

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

    /* Check default routing */
    /* Send expected to AudioHub */
    SendTrigger(c_stim_ah, 2);
    SendExpected(c_stim_ah, c_stim_de, modelOut, modelIn);

    for(int testIter = 0; testIter < TEST_ITERATIONS; testIter++)
    {
        /* Make a random update to the routing - route a random source to a random destination */
        unsigned map = testCmd[random_get_random_number(rg) % (sizeof(testCmd)/sizeof(testCmd[0]))];
        unsigned dst = random_get_random_number(rg) % CHANNEL_MAP_AUD_SIZE; // TODO this should be CHANNEL_MAP_USB_SIZE for SET_SAMPLES_TO_HOST_MAP
        unsigned src = random_get_random_number(rg) % (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT);

        switch(map)
        {
            case SET_SAMPLES_TO_DEVICE_MAP:
                debug_printf("Mapping output to AudioIF: %d", dst);
                PrintDestString(map, dst);
                debug_printf(" from %d", src);
                PrintSourceString(src);
                debug_printf("\n");

                /* Update the mixer */
                SendTrigger(c_stim_ah, 1);
                UpdateMixerOutputRouting(c_mix_ctl, map, dst, src);
                break;

            case SET_SAMPLES_TO_HOST_MAP:
                debug_printf("Mapping output to Host : %d", dst);
                PrintDestString(map, dst);
                debug_printf(" from %d", src);
                PrintSourceString(src);
                debug_printf("\n");

                /* Update the mixer */
                SendTrigger(c_stim_ah, 1);
                UpdateMixerOutputRouting(c_mix_ctl, map, dst, src);
                break;

            default:
                printstr("ERROR: Bad cmd in stim(): ");
                printintln(map);
              break;
        }

        /* Update the model */
        UpdateModel(modelOut, modelMixerOut, modelIn, map, dst, src);

        SendExpected(c_stim_ah, c_stim_de, modelOut, modelIn);

    }

    /* Send kill messages to Fake AudioHub & Fake Decouple */
    outct(c_stim_ah, XS1_CT_END);
    inct(c_stim_ah);

    outct(c_stim_de, XS1_CT_END);
    inct(c_stim_de);

    printstrln("PASS");
    exit(0);
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


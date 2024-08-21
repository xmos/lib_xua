// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* Tests that routing of mixer inputs behaves as expected
 *
 * The device supports MAX_MIX_COUNT mixers each with MIX_INPUTS inputs.
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
*/
#include <stdint.h>
#include <stddef.h>
#include "platform.h"
#include "xua.h"
#include "debug_print.h"
#include "assert.h"
#include "random.h"

#ifndef TEST_ITERATIONS
#define TEST_ITERATIONS (300)
#endif

#include "./../test_mixer_routing_output/src/mixer_test_shared.h"

struct ModelMixer
{
    uint32_t deviceMap[NUM_USB_CHAN_OUT];
    uint32_t hostMap[NUM_USB_CHAN_IN];
    uint32_t mixMap_input[MAX_MIX_COUNT];
    uint32_t mixMap_src[MAX_MIX_COUNT];
    uint32_t mixOutput[MAX_MIX_COUNT];
};

void InitModel(struct ModelMixer &modelMixer)
{
    for(size_t i = 0; i < NUM_USB_CHAN_OUT; i++)
    {
        modelMixer.deviceMap[i] = i;
    }

    for(size_t i = 0; i < NUM_USB_CHAN_IN; i++)
    {
        modelMixer.hostMap[i] = NUM_USB_CHAN_OUT+i;
    }

    for(size_t i = 0; i < MAX_MIX_COUNT; i++)
    {
        // This test only allows for one "active" input to each mixer
        modelMixer.mixMap_src[i] = i;
        modelMixer.mixMap_input[i] = i;

        uint32_t sample = i;
        SET_SOURCE(sample, SRC_HOST);
        SET_CHANNEL(sample, i);
        modelMixer.mixOutput[i] = sample;
    }
}

void GenExpectedSamples(struct ModelMixer &modelMixer,
                        uint32_t modelOut[NUM_USB_CHAN_OUT],
                        uint32_t modelIn[NUM_USB_CHAN_IN])
{
    /* First generate model mix outputs - run MIX tiles to allow mix inputs derived from mix outputs to propagate */
    for(int j = 0; j < MAX_MIX_COUNT; j++)
    {
        for(size_t i = 0; i < MAX_MIX_COUNT; i++)
        {
            int src = modelMixer.mixMap_src[i];
            modelMixer.mixOutput[i] = CreateSample(modelMixer.mixOutput, src);
        }
    }

    for(size_t i = 0; i<NUM_USB_CHAN_OUT; i++)
    {
        int src = modelMixer.deviceMap[i];
        modelOut[i] = CreateSample(modelMixer.mixOutput, src);
    }

    for(size_t i = 0; i<NUM_USB_CHAN_IN; i++)
    {
        int src = modelMixer.hostMap[i];
        modelIn[i] = CreateSample(modelMixer.mixOutput, src);
    }
}



void MapMixerInput(int mix, int input, int src, struct ModelMixer &modelMixer, chanend c_mix_ctl,
                        chanend c_stim_ah, chanend c_stim_de, uint32_t modelIn[], uint32_t modelOut[])
{
    debug_printf("Mapping mix %d input %d", mix, input);
    debug_printf(" from %d", src);
    PrintSourceString(src);
    debug_printf("\n");

    /* This test only allows for one input to travel "untouched" to the mix output - since this test doesn't model the actual mixing.
     * Because of this we must also mod the mixer weights, not just the mixer input map.
     * If we simply just apply an update to the mixer input mapping it would not produce an observable difference on the mixer output
     */

    /* Set previously "activated" input weight to 0 */
    debug_printf("Setting mix %d, weight %d to 0\n", mix, modelMixer.mixMap_input[mix]);
    SendTrigger(c_stim_ah, 1);
    UpdateMixerWeight(c_mix_ctl, mix, modelMixer.mixMap_input[mix], 0);

/* Set new "activated" input wright to max (i.e. x1) */
    debug_printf("Setting mix %d, weight %d to %x\n", mix, input, XUA_MIXER_MAX_MULT);
    SendTrigger(c_stim_ah, 1);
    UpdateMixerWeight(c_mix_ctl, mix, input, XUA_MIXER_MAX_MULT);

    /* Update mixer input in model */
    modelMixer.mixMap_src[mix] = src;
    modelMixer.mixMap_input[mix] = input;

    /* Run twice to allow mix inputs derived from mix outputs to propagate */
    GenExpectedSamples(modelMixer, modelOut, modelIn);

    /* Finally update the acutal mixer input map */
    SendTrigger(c_stim_ah, 1);
    UpdateMixMap(c_mix_ctl, mix, input, src);

    SendTrigger(c_stim_ah, 1);

    SendExpected(c_stim_ah, c_stim_de, modelOut, modelIn);
}


/* This task configures the routing and maintains a model of the expected routing output
 * it provides this to the Fake AudioHub and Fake Decouple tasks such that they can self check
 */
void stim(chanend c_stim_ah, chanend c_stim_de, chanend c_mix_ctl)
{
    uint32_t modelOut[NUM_USB_CHAN_OUT];
    uint32_t modelIn[NUM_USB_CHAN_IN];

    random_generator_t rg = random_create_generator_from_seed(TEST_SEED);

    struct ModelMixer modelMixer;

    InitModel(modelMixer);

    GenExpectedSamples(modelMixer, modelOut, modelIn);

    /* There is single sample delay between the two mixer cores, so trigger twice to flush though a block
     * of zero samples */
    SendTrigger(c_stim_ah, 2);

    /* Send expected samples to AH and DE and run checks */
    SendExpected(c_stim_ah, c_stim_de, modelOut, modelIn);

    /* Firstly route mixer outputs to the audio interfaces (we could have chosen host)
     * such that we can observe and check the outputs from the mixer
     */
    for(size_t i = 0; i < MAX_MIX_COUNT; i++)
    {
        int map = SET_SAMPLES_TO_DEVICE_MAP;
        assert(i < NUM_USB_CHAN_OUT);
        int dst = i;
        int src = NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN+i; // mix0, mix1..
        debug_printf("Mapping output to AudioIF: %d ", dst);

        PrintDestString(map, dst);
        debug_printf(" from %d", src);
        PrintSourceString(src);
        debug_printf("\n");

        SendTrigger(c_stim_ah, 1);

        /* Update the mixer */
        UpdateMixerOutputRouting(c_mix_ctl, map, dst, src);

        /* Update the model */
        modelMixer.deviceMap[dst] = src;
    }

    /* Send expected samples to fake AudioHub and Decouple for checking */
    SendExpected(c_stim_ah, c_stim_de, modelOut, modelIn);

    for(int testIter = 0; testIter < TEST_ITERATIONS; testIter++)
    {
        /* Make a random update to the routing - route a random source to a random mix input */
        unsigned mix = random_get_random_number(rg) % MAX_MIX_COUNT;
        unsigned input = random_get_random_number(rg) % MIX_INPUTS;

        /* Note, we don't currently support a mix input dervived from another mix
         * This is not trivial to test since the current mixer implementation only allows for one
         * config update per "trigger"
         */
        unsigned src = random_get_random_number(rg) % NUM_USB_CHAN_IN + NUM_USB_CHAN_OUT;

        debug_printf("Iteration: %d\n", testIter);
        MapMixerInput(mix, input, src, modelMixer, c_mix_ctl, c_stim_ah, c_stim_de, modelIn, modelOut);
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


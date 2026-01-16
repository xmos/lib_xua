// Copyright 2025-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* This is a simple test that checks that the in stream exits and re-enters underflow as expected.
 *  Currently operates at 'high-speed' sized packets only
 */

#include <platform.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#define XASSERT_UNIT MAIN
#include "xassert.h"

#include "xua.h"
#include "xua_commands.h"
#include "xc_ptr.h"

#include "debug_print.h"

#include "random.h"
#include "test_seed.h"
#ifndef TEST_SEED
#error TEST_SEED not defined
#endif

/* From xua_audiohub.xc */
extern unsigned samplesOut[NUM_USB_CHAN_OUT];
extern unsigned samplesIn[2][NUM_USB_CHAN_IN];
#include "xua_audiohub_st.h"

#define TEST_SAMPLE_RATE_HZ            (DEFAULT_FREQ)
#define TEST_USB_FRAME_RATE_HZ         (8000) //HS
#define SUBSLOT_SIZE_BYTES             (4)
#define PACKET_SAMPLES_PER_CHAN        (TEST_SAMPLE_RATE_HZ/TEST_USB_FRAME_RATE_HZ)
#define PACKET_SIZE_NOMINAL_BYTES      (PACKET_SAMPLES_PER_CHAN * SUBSLOT_SIZE_BYTES) * (NUM_USB_CHAN_IN)

#define CMD_CHECK_UNDERFLOW            (1)
#define CMD_CHECK_RAMP                 (2)
#define CMD_GEN_RAMP                   (3)
#define CMD_DIE                        (4)

void GetADCCounts(unsigned samFreq, int &min, int &mid, int &max);

/* From decouple.xc */
extern unsigned audioBuffIn[];

int g_inEpReady = 0;
int g_inEpLength = 0;
unsafe
{
    volatile int * unsafe gp_inEpReady = &g_inEpReady;
    volatile int * unsafe gp_inEpLength = &g_inEpLength;
    volatile unsigned * unsafe gp_pktBuffer;
}

#define DELAY 2000
static inline void Trigger(chanend c, unsigned cmd, int count)
{
    timer t;
    unsigned time;
    for(int i = 0; i < count; i++)
    {
        c <: cmd;
        t :> time;
        t when timerafter(time+DELAY) :> void;
    }
}

/* Buffering uses this to size packets */
extern unsigned g_speed;
/* Fill buffer enough to bring it out of Underflow*/
{int, int} FillBuffer(chanend c_stim_ep, chanend c_stim_au, random_generator_t rg, int pktSizes[64], int &currentPktSamples)
{
    int nextPktSamples;
    int pktSize;
    int pktCount = 0;
    int fillLevel = 0;

    int min, mid, max;
    GetADCCounts(DEFAULT_FREQ, min, mid, max);
    const int min_pkt_size = ((min * SUBSLOT_SIZE_BYTES * NUM_USB_CHAN_IN + 3) & ~0x3) + 4;

    while(1)
    {
        /* Setup next packet sizing */
        /* Note, technically we should not be doing +/- for freqs like 44.1 but for this test it's a 'don't care' */
        int pktSamplesAdjust = (int) (random_get_random_number(rg) % 3) - 1; // Rand number between -1 and 1
        nextPktSamples = PACKET_SAMPLES_PER_CHAN + pktSamplesAdjust;
        g_speed = (nextPktSamples << 16);

        /* E.g. We expect 6 +/- 1 transfers per packet at 48000 */
        Trigger(c_stim_au, CMD_GEN_RAMP, currentPktSamples);

        /* Keep out own record of buffer fill level */
        pktSize = (currentPktSamples * NUM_USB_CHAN_IN * SUBSLOT_SIZE_BYTES);
        fillLevel += pktSize + 4; /* +4 since store packet length in buffer */

        /* Record packet sizes for when we are draining the buffer later */
        pktSizes[pktCount++] = pktSize;
        assert(pktCount < sizeof(pktSizes)/sizeof(pktSizes[0]));

        currentPktSamples = nextPktSamples;
        /* Stop filling the buffer when we are taking it out of underflow */
        if ((fillLevel >= (min_pkt_size*2)) && (fillLevel < (min_pkt_size*3)))
        {
            return {pktCount, fillLevel};
        }

        /* Check that the buffer has remained in underflow */
        Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 1);
    }
}

int stim(chanend c_stim_ep, chanend c_stim_au)
{
    int pktSamplesPerChan = PACKET_SAMPLES_PER_CHAN;
    int fillLevel = 0;
    int pktCount = 0;
    int pktSizes[64];

    /* We need to set feedback value such that decouple appropriately sized packets */
    g_speed = (pktSamplesPerChan << 16);

    random_generator_t rg = random_create_generator_from_seed(TEST_SEED);

    /* Initially we should be in underflow */
    Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 1);

    /* Give the buffering some recorded samples until we come out of underflow */
    {pktCount, fillLevel} = FillBuffer(c_stim_ep, c_stim_au, rg, pktSizes, pktSamplesPerChan);

    /* Will take a packet send in order for buffer to trigger the underflow check */
    Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 1);

    /* Subtract the amount of bytes used to store packet lengths */
    fillLevel -= (pktCount * sizeof(unsigned));

    /* Now check the packets/samples pop out as expected */
    for(int i = 0; i < pktCount; i++)
    {
        Trigger(c_stim_ep, CMD_CHECK_RAMP, 1);
        fillLevel -= (pktSizes[i]);
    }
    assert(fillLevel == 0);

    /* Check we're into underflow after draining the buffer */
    Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 1);
    Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 1);

    /* Fill the buffer up again */
    {pktCount, fillLevel} = FillBuffer(c_stim_ep, c_stim_au, rg, pktSizes, pktSamplesPerChan);

    /* Will take a packet send in order for buffer to triffer the underflow check */
    Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 1); // After this is completed the decoupler should no longer be in underflow

    /* Fill a bit more .. */
    Trigger(c_stim_au, CMD_GEN_RAMP, pktSamplesPerChan);
    fillLevel -= (pktCount * sizeof(unsigned));

    /* Now check the packets pop out as expected */
    int i = 0;
    while(fillLevel > 0)
    {
        Trigger(c_stim_ep, CMD_CHECK_RAMP, 1);
        fillLevel -= (pktSizes[i++]);
    }

    Trigger(c_stim_ep, CMD_CHECK_RAMP, 1);
    /* annnnd back into underflow...*/
    Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 3);
    timer t;
    unsigned time;
    t :> time;
    /* Kill decouple */
    SET_SHARED_GLOBAL(g_streamChange_flag, XUA_EXIT);

    t when timerafter(time + 1000) :> void;
    /* kill the test cores */
    c_stim_au <: CMD_DIE;
    c_stim_ep <: CMD_DIE;

    int tmp = XUA_EXIT;
    while(tmp != XUA_AUDCTL_NO_COMMAND)
        GET_SHARED_GLOBAL(tmp, g_streamChange_flag);

    printstr("PASS\n");
    return 0;
}

/* Generates a simple ramp as audio data */
int Fake_XUA_AudioHub(chanend c_aud, chanend c_stim)
{
    int readBuffNo = 0;
    unsigned underflowWord = 0;
    int cmd;
    int ramp = 100;
    timer t;
    unsigned time;
    t :> time;

    // At the very start, a XUA_AUD_SET_AUDIO_START is expected from decoupler to set things going
    t when timerafter(time + 1000) :> void; // Give decoupler time to start
    unsigned command = DoSampleTransfer(c_aud, readBuffNo, underflowWord);
    assert (command == XUA_AUD_SET_AUDIO_START && msg("Unexpected command"));
    int dsdMode = inuint(c_aud);
    int curSamRes_DAC = inuint(c_aud);
    outct(c_aud, XS1_CT_END);


    while(1)
    {
        c_stim :> cmd;

        if(cmd == CMD_DIE)
        {
            outct(c_aud, XS1_CT_END);
            chkct(c_aud, XS1_CT_END);
            return 0;
        }
        else
        {
            for(int i = 0; i < NUM_USB_CHAN_IN; i++)
            {
                samplesIn[readBuffNo][i] = (ramp++);
            }
            unsigned command = DoSampleTransfer(c_aud, readBuffNo, underflowWord);
            readBuffNo = !readBuffNo;
        }
    }

    return 0;
}

/* This will be called by decouple */
XUD_Result_t XUD_SetReady_InPtr(XUD_ep ep, unsigned addr, int length)
{
    unsafe
    {
        assert(*gp_inEpReady == 0 && msg("EP already marked ready"));
        gp_pktBuffer = (unsigned * unsafe) addr;
        *gp_inEpLength = length;
        *gp_inEpReady = 1;
    }

    return XUD_RES_OKAY;
}


/* Checks the contents of audio packets based on the info received from the stim() task */
int Fake_XUA_Buffer_Ep(chanend c_stim, chanend c_aud_ctl)
{
    timer t;
    uint32_t time;
    t :> time;
    int length;
    int cmd;
    int ramp = 100; /* Start at something other than 0 just to avoid confusing with underflow */

    /* Decouple waits for this to be set before doing anything. It then resets it */
    SET_SHARED_GLOBAL(g_aud_to_host_flag, 1);

    /* Decouple won't mark the IN EP ready to send data until it receives a SET_STREAM_FORMAT_IN */
    // TODO change to SET_SAMPLE_RATE?
    SET_SHARED_GLOBAL(g_formatChange_NumChans, NUM_USB_CHAN_IN);
    SET_SHARED_GLOBAL(g_formatChange_SubSlot, SUBSLOT_SIZE_BYTES);
    SET_SHARED_GLOBAL(g_formatChange_SampRes, SUBSLOT_SIZE_BYTES * 4);
    SET_SHARED_GLOBAL(g_streamChange_flag, XUA_AUDCTL_SET_STREAM_INPUT_START);

    /* Note this would normally be received by the Endpoint0 task */
    chkct(c_aud_ctl, XS1_CT_END);

    while(1)
    {
        c_stim :> cmd;

        if(cmd == CMD_DIE)
            return 0;

        unsafe
        {
            /* Wait for decouple to mark EP as ready to send */
            while(!*gp_inEpReady);
                *gp_inEpReady = 0;

            /* Grab packet length from the global stored in our implemetation of XUD_SetReady_InPtr() */
            length = *gp_inEpLength;
        }

        unsafe
        {
            for(int i = 0; i < length/sizeof(int); i++)
            {
                int checkValue = 0;

                assert(((cmd == CMD_CHECK_RAMP) || (cmd == CMD_CHECK_UNDERFLOW)) && msg("Bad command"));

                if(cmd == CMD_CHECK_RAMP)
                {
                    checkValue = ramp;
                }

                if(*(gp_pktBuffer+i) != checkValue)
                {
                    debug_printf("ERR BUFFER_EP: expected %d got %d from %d. cmd %d\n", checkValue, *(gp_pktBuffer+i), (int) (gp_pktBuffer+i), cmd);
                }
                assert(*(gp_pktBuffer+i) == checkValue && msg("Bad value in buffer"));

                ramp += (cmd == CMD_CHECK_RAMP);
            }
        }

        /* Wait some time emulating delay before sending packet */
        t when timerafter(time + 100) :> void;

        /* Sync with decouple thread */
        SET_SHARED_GLOBAL(g_aud_to_host_flag, 1);
    }

    return 0;
}

/* Wrapper for decouple just to save off chanend */
void DecoupleWrapper(chanend c_aud_ctl, chanend c_aud)
{
    asm("stw %0, dp[buffer_aud_ctl_chan]"::"r"(c_aud_ctl));
    XUA_Buffer_Decouple(c_aud);
}

int main(void)
{
    chan c_stim_ep;
    chan c_stim_au;
    chan c_aud;

    chan c_aud_ctl;

    par
    {
        on tile[0]:
        {
            par
            {
                Fake_XUA_Buffer_Ep(c_stim_ep, c_aud_ctl);
                DecoupleWrapper(c_aud_ctl, c_aud);
                Fake_XUA_AudioHub(c_aud, c_stim_au);
                stim(c_stim_ep, c_stim_au);
            }
        }
    }

    return 0;
}

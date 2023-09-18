// Copyright 2023 XMOS LIMITED.
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

/* From decouple.xc */
extern unsigned g_aud_from_host_flag;
extern unsigned audioBuffIn[];

int g_inEpReady = 0;
int g_inEpLength = 0;
unsafe
{
    volatile int * unsafe gp_inEpReady = &g_inEpReady;
    volatile int * unsafe gp_inEpLength = &g_inEpLength;
    volatile unsigned * unsafe gp_pktBuffer;
}


#define DELAY 1000
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


int stim(chanend c_stim_ep, chanend c_stim_au)
{
    random_generator_t rg = random_create_generator_from_seed(TEST_SEED);


    /* Initially we should be in underflow */
    Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 1);

    /* Give the buffering some some recorded samples until we come out of underflow */

    /* Each packet uses PACKET_SIZE_NOMIMAL_BYTES in the buffer + 4 bytes to store the packet length
     * it will therefore the number of packets to fill buffer is IN_BUFFER_PREFILL/(PACKET_SIZE_NOMINAL_BYTES + 4)
     * In the case of 48kHz this is 1560/52 = 30
     *
     * Note, this all assumes nominal packet size only...
     */
    const int prefillPackets = IN_BUFFER_PREFILL/(PACKET_SIZE_NOMINAL_BYTES+4);

    int fillLevel = 0;

    while(fillLevel < IN_BUFFER_PREFILL)
    {
        /* Note, techically we shoudl be doing +/- for freqs like 44.1 but for this test it's a 'don't care' */
        int pktSizeAdjust = (int) (random_get_random_number(rg) % 3) - 1 ; // Rand number between -1 and 1

        /* E.g. We expect 6 +/- 1 transfers per packet at 48000 */
        Trigger(c_stim_au, CMD_GEN_RAMP, PACKET_SAMPLES_PER_CHAN + pktSizeAdjust);
        Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 1);

        fillLevel += (PACKET_SAMPLES_PER_CHAN * NUM_USB_CHAN_IN * SUBSLOT_SIZE_BYTES) + 4;
        fillLevel += (NUM_USB_CHAN_IN * pktSizeAdjust);
    }

    /* Now check the packets pop out as expected */
    Trigger(c_stim_ep, CMD_CHECK_RAMP, prefillPackets);

    /* Check we're into underflow after draining the buffer */
    Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 1);

    Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 1);

    /* Fill the buffer up again */
    for(int i = 0; i < prefillPackets; i++)
    {
        /* We expect 6 transfers per packet at 48000 */
        Trigger(c_stim_au, CMD_GEN_RAMP, PACKET_SAMPLES_PER_CHAN);
        Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 1);
    }

    /* Fill a bit more .. */
    for(int i = 0; i < 2; i++)
    {
        /* We expect 6 transfers per packet at 48000 */
        Trigger(c_stim_au, CMD_GEN_RAMP, 6);
    }

    /* Now check the packets pop out as expected */
    Trigger(c_stim_ep, CMD_CHECK_RAMP, prefillPackets + 2);

    /* annnnd back into underflow...*/
    Trigger(c_stim_ep, CMD_CHECK_UNDERFLOW, 3);

    c_stim_au <: CMD_DIE;
    printstr("PASS\n");
    exit(0);

    return 0;
}

/* Generates a simple ramp as audio data */
int Fake_XUA_AudioHub(chanend c_aud, chanend c_stim)
{
    int readBuffNo = 0;
    unsigned underflowWord = 0;
    int cmd;
    int ramp = 100;

    while(1)
    {
        c_stim :> cmd;

        if(cmd == CMD_DIE)
        {
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
    assert(length == PACKET_SIZE_NOMINAL_BYTES && msg("For this test we expect nominal length packets"));
    unsafe
    {
        assert(*gp_inEpReady == 0 && msg("EP already marked ready"));
        gp_pktBuffer = (unsigned * unsafe) addr;
        *gp_inEpLength = length;
        *gp_inEpReady = 1;
    }

    return XUD_RES_OKAY;
}

extern unsigned g_speed;

/* Checks the contents of audio packets based on the info received from the stim() task */
int Fake_XUA_Buffer_Ep(chanend c_stim, chanend c_aud_ctl)
{
    timer t;
    uint32_t time;
    t :> time;
    int length;
    int cmd;
    int ramp = 100; /* Start at something other than 0 just to avoid confusing with underflow */

    /* We need to set feedback value such that decouple appropriately sized packets */
    g_speed = (PACKET_SAMPLES_PER_CHAN << 16);

    /* Decouple waits for this to be set before doing anything. It then resets it */
    SET_SHARED_GLOBAL(g_aud_to_host_flag, 1);

    /* Decouple won't mark the IN EP ready to send data until it receives a SET_STREAM_FORMAT_IN */
    // TODO change to SET_SAMPLE_RATE?
    SET_SHARED_GLOBAL(g_formatChange_NumChans, NUM_USB_CHAN_IN);
    SET_SHARED_GLOBAL(g_formatChange_SubSlot, SUBSLOT_SIZE_BYTES);
    SET_SHARED_GLOBAL(g_formatChange_SampRes, SUBSLOT_SIZE_BYTES * 4);
    SET_SHARED_GLOBAL(g_freqChange_flag, SET_STREAM_FORMAT_IN);

    /* Note this would normally be received by the Endpoint0 task */
    chkct(c_aud_ctl, XS1_CT_END);

    while(1)
    {
        c_stim :> cmd;

        unsafe
        {
            /* Wait for decouple to mark EP as ready to send */
            while(!*gp_inEpReady);
                *gp_inEpReady = 0;

            /* Grab packet length from the global stored in our implemetation of XUD_SetReady_InPtr() */
            length = *gp_inEpLength;
        }

        assert(length == PACKET_SIZE_NOMINAL_BYTES && msg("Unexpcted length"));

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
                    debug_printf("BUFFER_EP: expected %d got %d from %d\n", checkValue, *(gp_pktBuffer+i), (int) (gp_pktBuffer+i));
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

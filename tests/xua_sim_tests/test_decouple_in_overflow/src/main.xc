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

/* From decouple.xc */
extern int g_aud_to_host_fill_level;
extern xc_ptr g_aud_to_host_wrptr;
extern xc_ptr g_aud_to_host_rdptr;
extern xc_ptr aud_to_host_fifo_start;
extern xc_ptr aud_to_host_fifo_end;

#define TEST_SAMPLE_RATE_HZ            (DEFAULT_FREQ)
#define TEST_USB_FRAME_RATE_HZ         (8000) //HS
#define SUBSLOT_SIZE_BYTES             (4)
#define PACKET_SAMPLES_PER_CHAN        (TEST_SAMPLE_RATE_HZ/TEST_USB_FRAME_RATE_HZ)
#define PACKET_SIZE_NOMINAL_BYTES      (PACKET_SAMPLES_PER_CHAN * SUBSLOT_SIZE_BYTES) * (NUM_USB_CHAN_IN)

#define CMD_CHECK_UNDERFLOW            (1)
#define CMD_CHECK_RAMP                 (2)
#define CMD_CHECK_RAMP_START_OFFSET    (3)
#define CMD_GEN_RAMP                   (4)
#define CMD_DIE                        (5)

#define RAMP_START_VALUE               (100)

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
static inline void Trigger_audiohub(chanend c, unsigned cmd, int count)
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

static inline void Trigger_EP_Buffer(chanend c, unsigned cmd)
{
    timer t;
    unsigned time;
    c <: cmd;
    t :> time;
    t when timerafter(time+DELAY) :> void;
}

static inline void Trigger_EP_Buffer_ramp_start_offset(chanend c, unsigned cmd, int start)
{
    timer t;
    unsigned time;
    c <: cmd;
    c <: start;
    t :> time;
    t when timerafter(time+DELAY) :> void;
}


/* Buffering uses this to size packets */
extern unsigned g_speed;

int stim(chanend c_stim_ep, chanend c_stim_au)
{
    int pktSamplesPerChan = PACKET_SAMPLES_PER_CHAN;
    int total_samples_written_per_channel = 0;
    int currentPktSamples = pktSamplesPerChan;
    int prev_fill_level = 0;
    int nextPktSamples;
    unsigned rdptr, wrptr, start, end;
    timer t;
    unsigned time;
    t :> time;

    /* We need to set feedback value such that decouple appropriately sized packets */
    g_speed = (pktSamplesPerChan << 16);

    random_generator_t rg = random_create_generator_from_seed(TEST_SEED);

    time += 3000; // Large delay to give all the threads time to start
    t when timerafter(time) :> time;

    GET_SHARED_GLOBAL(start, aud_to_host_fifo_start);
    GET_SHARED_GLOBAL(end, aud_to_host_fifo_end);
    GET_SHARED_GLOBAL(rdptr, g_aud_to_host_rdptr);
    GET_SHARED_GLOBAL(wrptr, g_aud_to_host_wrptr);

    debug_printf("Buffer start: %u, end: %u, rdptr: %u, wrptr: %u\n", start, end, rdptr, wrptr);

    unsigned aud_to_host_buf_fill_level;

    for(int iter=0; iter<2; iter++)
    {
        printstr("Iter ");
        printintln(iter);

        // Empty -> overflow -> handle overflow
        while(1)
        {
            /* Setup next packet sizing */
            /* Note, technically we should not be doing +/- for freqs like 44.1 but for this test it's a 'don't care' */
            int pktSamplesAdjust = (int) (random_get_random_number(rg) % 3) - 1; // Rand number between -1 and 1
            nextPktSamples = PACKET_SAMPLES_PER_CHAN + pktSamplesAdjust;
            g_speed = (nextPktSamples << 16); // This has to be set before sending the current frame since empty space is made for the next packet at the end of the current packet itself.

            /* E.g. We expect 6 +/- 1 transfers per packet at 48000 */
            Trigger_audiohub(c_stim_au, CMD_GEN_RAMP, currentPktSamples);

            total_samples_written_per_channel += currentPktSamples;

            currentPktSamples = nextPktSamples;
            GET_SHARED_GLOBAL(aud_to_host_buf_fill_level, g_aud_to_host_fill_level);

            if(prev_fill_level > aud_to_host_buf_fill_level)
            {
                // After writing the current packet, buffer fill level has fallen instead of risen => overflow detected and handled in handle_audio_request()
                printstrln("handle_audio_request: Overflow detected and handled");
                break;
            }
            prev_fill_level = aud_to_host_buf_fill_level;
        }

        // Verify that overflow recovery retains only the 2 most recent packets in the buffer.

        int total_samples_written = total_samples_written_per_channel * NUM_USB_CHAN_IN; // Total samples written by Audiohub into the buffer
        unsafe {
            debug_printf("Fill level after handling overflow = %d, full fill level = %d\n", aud_to_host_buf_fill_level, total_samples_written);
        }

        // Convert buffer fill (in bytes) to number of data samples.
        // Each sample is 4 bytes. Two entries are reserved for packet length metadata.
        int post_overflow_recovery_samples_in_buf = (aud_to_host_buf_fill_level/4) - 2;

        // Ensure the buffer contains only the 2 most recent packets after recovery.
        // Calculate the first expected sample to be read after recovery.
        int first_expected_read_sample = RAMP_START_VALUE + (total_samples_written - post_overflow_recovery_samples_in_buf);

        printstr("Read till buffer empty\n");

        // Confirm underflow occurs after reading exactly 2 packets, validating correct overflow recovery.
        Trigger_EP_Buffer(c_stim_ep, CMD_CHECK_UNDERFLOW); // Initially we should be in underflow

        Trigger_EP_Buffer_ramp_start_offset(c_stim_ep, CMD_CHECK_RAMP_START_OFFSET, first_expected_read_sample); // Check first packet

        Trigger_EP_Buffer(c_stim_ep, CMD_CHECK_RAMP); // Check second packet

        Trigger_EP_Buffer(c_stim_ep, CMD_CHECK_UNDERFLOW); // Back in underflow

        // Buffer fill level should be 0 now
        GET_SHARED_GLOBAL(rdptr, g_aud_to_host_rdptr);
        GET_SHARED_GLOBAL(wrptr, g_aud_to_host_wrptr);

        assert (rdptr == wrptr && msg("Buffer not empty when expected to be so"));
        printstr("Buffer empty\n");

        prev_fill_level = 0;
    }

    printstr("End test\n");
    // End the test
    t :> time;
    t when timerafter(time + 1000) :> time;
    /* Kill decouple */
    SET_SHARED_GLOBAL(g_streamChange_flag, XUA_EXIT);

    t when timerafter(time + 100) :> void;
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
    int ramp = RAMP_START_VALUE; /* Start at something other than 0 just to avoid confusing with underflow */

    timer t;
    unsigned time;
    t :> time;

    // At the very start, a XUA_AUD_SET_AUDIO_START is expected from decoupler to set things going
    t when timerafter(time + 100) :> void; // Give decoupler time to start
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
    int ramp = RAMP_START_VALUE; /* Start at something other than 0 just to avoid confusing with underflow */

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
        if(cmd == CMD_CHECK_RAMP_START_OFFSET)
        {
            c_stim :> ramp; // overwrite the point we want to start checking from
        }

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

                assert(((cmd == CMD_CHECK_RAMP) || (cmd == CMD_CHECK_UNDERFLOW) || (cmd == CMD_CHECK_RAMP_START_OFFSET)) && msg("Bad command"));

                if((cmd == CMD_CHECK_RAMP) || (cmd == CMD_CHECK_RAMP_START_OFFSET))
                {
                    checkValue = ramp;
                }

                if(*(gp_pktBuffer+i) != checkValue)
                {
                    debug_printf("ERR BUFFER_EP: expected %d got %d from %d. cmd %d\n", checkValue, *(gp_pktBuffer+i), (int) (gp_pktBuffer+i), cmd);
                }
                assert(*(gp_pktBuffer+i) == checkValue && msg("Bad value in buffer"));

                ramp += ((cmd == CMD_CHECK_RAMP) || (cmd == CMD_CHECK_RAMP_START_OFFSET));
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

/**
 * Test: Buffer overflow and recovery
 *
 * Sequence:
 * - Write into an empty buffer until it overflows; overflow is handled internally by handle_audio_request().
 * - Verify that overflow handling preserves the expected data (2 latest packets) in the buffer.
 * - Read from the buffer until it underflows (empty).
 * - Repeat the sequence.
 *
 * Transition:
 * empty → overflow → handle overflow → empty → overflow → handle overflow → empty
 *
 * Notes:
 * Run the sequence twice to cover both cases:
 * 1. Start from buffer empty with rdptr = wrptr = start
 * 2. Start from buffer empty with rdptr = wrptr ≠ start
 */
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

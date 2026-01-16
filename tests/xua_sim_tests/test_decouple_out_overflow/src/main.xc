// Copyright 2025-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* This is a simple test that checks that the out stream exits and re-enters underflow as expected */

#include <platform.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#define XASSERT_UNIT MAIN
#include "xassert.h"

#include "xua.h"
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

/* From decoupler.xc */
extern unsigned outOverflow;
extern unsigned outUnderflow;
extern unsigned g_aud_from_host_flag;
extern unsigned outAudioBuff[(BUFF_SIZE_OUT >> 2)+ (MAX_DEVICE_AUD_PACKET_SIZE_OUT >> 2)];
extern xc_ptr g_aud_from_host_rdptr;
extern xc_ptr g_aud_from_host_wrptr;
extern xc_ptr aud_from_host_fifo_start;
extern xc_ptr aud_from_host_fifo_end;

#define TEST_SAMPLE_RATE_HZ            (DEFAULT_FREQ)
#define TEST_USB_FRAME_RATE_HZ         (8000) //HS
#define SUBSLOT_SIZE_BYTES             (4)
#define PACKET_SAMPLES_PER_CHAN        (TEST_SAMPLE_RATE_HZ/TEST_USB_FRAME_RATE_HZ)
#define PACKET_SIZE_NOMINAL_BYTES      (PACKET_SAMPLES_PER_CHAN * SUBSLOT_SIZE_BYTES) * (NUM_USB_CHAN_OUT)

#define UNDERFLOW_WORD                 (0xBADDA55)

typedef enum
{
    CHECK_STATIC = 100,
    CHECK_RAMP = 101,
} CheckType_t;

typedef enum
{
    GEN_RAMP = 200,
} GenType_t;

int CheckBuffer(unsigned buffer[length], size_t length, CheckType_t checkType, unsigned checkData)
{
    switch(checkType)
    {
        case CHECK_STATIC:
            for(size_t i = 0; i < length; i++)
            {
                if(buffer[i] != checkData)
                {
                    debug_printf("Expected %d (%x) got %d (%x)\n", checkData, checkData, buffer[i], buffer[i]);
                    assert(buffer[i] == checkData);
                }
            }
           break;

        case CHECK_RAMP:

            for(size_t i = 0; i < length; i++)
            {
                if(buffer[i] != checkData)
                {
                    debug_printf("Expected %d (%x) got %d (%x)\n", checkData, checkData, buffer[i], buffer[i]);
                    assert(buffer[i] == checkData);
                }
                checkData++;
            }
           break;

        default:
            assert(0 && msg("Bad command in CheckBuffer"));
            break;
    }
}

static int total_size_written = 0; // total words written to the buffer
static inline int get_packet_size(void)
{
    /* Note, technically we shouldn't be doing +/- for freqs like 44.1 but for this test it's a 'don't care' */
    random_generator_t rg = random_create_generator_from_seed(TEST_SEED);
    //int pktSizeAdjust = (int) (random_get_random_number(rg) % 3) - 1 ; // Rand number between -1 and 1
    int pktSizeAdjust = 0;

    pktSizeAdjust *= (NUM_USB_CHAN_OUT * SUBSLOT_SIZE_BYTES);
    int pkt_size = (PACKET_SIZE_NOMINAL_BYTES + pktSizeAdjust);
    total_size_written += (pkt_size/sizeof(int));
    return pkt_size;
}

#define STIM_DELAY (1000)
int stim(chanend c_stim_ep, chanend c_stim_au)
{
    timer t;
    unsigned time;
    t :> time;
    int rampCheck = 0;
    unsigned rdptr, wrptr, start, end;

    random_generator_t rg = random_create_generator_from_seed(TEST_SEED);

    time += 3000; // Large delay to give all the threads time to start
    t when timerafter(time) :> void;

    unsigned out_overflow_val;
    GET_SHARED_GLOBAL(out_overflow_val, outOverflow);

    GET_SHARED_GLOBAL(start, aud_from_host_fifo_start);
    GET_SHARED_GLOBAL(end, aud_from_host_fifo_end);

    //debug_printf("Buffer start: %u, end: %u, actual_end %u\n", start, end, start+sizeof(outAudioBuff));

    /* Start from empty and write till buffer goes into overflow */
    while(out_overflow_val != 1)
    {
        int pktSize = get_packet_size();

        c_stim_ep <: (int) GEN_RAMP;
        c_stim_ep <: (int) pktSize;

        // Wait sometime to give decoupler time to process the packet, before reading outOverflow
        time += STIM_DELAY;
        t when timerafter(time) :> void;

        GET_SHARED_GLOBAL(out_overflow_val, outOverflow);
    }
    // print the status of the buffer
    GET_SHARED_GLOBAL(rdptr, g_aud_from_host_rdptr);
    GET_SHARED_GLOBAL(wrptr, g_aud_from_host_wrptr);

    // When overflowing when starting from empty:
    assert(wrptr > rdptr && msg("buffer overflow: wrptr not ahead of rdptr")); // wrptr should be ahead of rdptr
    assert(wrptr <= end && msg("buffer overflow: wrptr not within aud_from_host_fifo_end")); // wrptr should be within aud_from_host_fifo_end. (so should be within BUFF_SIZE_OUT and not in the extra packet space beyond BUFF_SIZE_OUT)

    /* Read till buffer out of overflow */

    // At this point write has detected buffer overflow and is paused. Read has not yet started so the buffer is still in its
    // initialised underflow state.
    // An overflow and underflow happening simultaneously is only acceptable when read hasn't started yet (rdptr = start)

    // Triggering a read now should get the underflow word but the very next read should start reading from the buffer, indicating that
    // read is out of underflow
    c_stim_au <: (int)CHECK_STATIC;
    c_stim_au <: (int)UNDERFLOW_WORD;

    t :> time;
    time += STIM_DELAY;
    t when timerafter(time) :> void;

     // Keep reading till buffer comes out of overflow
     while(out_overflow_val == 1)
     {
        c_stim_au <: (int)CHECK_RAMP;
        c_stim_au <: (int)rampCheck;
        rampCheck += (NUM_USB_CHAN_OUT); // Note, AudioHub receives in NUM_USB_CHAN_OUT sized blocks

        time += STIM_DELAY;
        t when timerafter(time) :> void;

        // Wait sometime before reading outOverflow
        GET_SHARED_GLOBAL(out_overflow_val, outOverflow);
     }

     // print the status of the buffer
     //GET_SHARED_GLOBAL(rdptr, g_aud_from_host_rdptr);
     //GET_SHARED_GLOBAL(wrptr, g_aud_from_host_wrptr);
     //debug_printf("After coming out of overflow, buffer status = rdptr %u, wrptr %u\n", rdptr, wrptr);

    /* Write till buffer in overflow again */
    // Now start writing again till write detects overflow. This time it should have wrapped the wrptr and stopped
    // somewhere before rdptr.
    while(out_overflow_val == 0)
    {
        int pktSize = get_packet_size();

        c_stim_ep <: (int) GEN_RAMP;
        c_stim_ep <: (int) pktSize;

        // Giver decoupler time to process the packet before reading outOverflow
        time += STIM_DELAY;
        t when timerafter(time) :> void;

        GET_SHARED_GLOBAL(out_overflow_val, outOverflow);
    }

    GET_SHARED_GLOBAL(rdptr, g_aud_from_host_rdptr);
    GET_SHARED_GLOBAL(wrptr, g_aud_from_host_wrptr);
    // When overflowing when rdptr != start
    assert(rdptr != start && msg("buffer overflow: rdptr unexpectedly at start")); // rdptr should not be at start
    assert(wrptr < rdptr && msg("buffer overflow: wrptr ahead of rdptr")); // wrptr should be behind of rdptr

    t :> time;
    time += STIM_DELAY;
    t when timerafter(time) :> void;

    /* Read till buffer comes out of overflow */
    while(out_overflow_val == 1)
    {
        c_stim_au <: (int)CHECK_RAMP;
        c_stim_au <: (int)rampCheck;
        rampCheck += (NUM_USB_CHAN_OUT); // Note, AudioHub receives in NUM_USB_CHAN_OUT sized blocks

        time += STIM_DELAY;
        t when timerafter(time) :> void;

        // Wait sometime before reading outOverflow
        GET_SHARED_GLOBAL(out_overflow_val, outOverflow);
    }

    // After coming out of overflow, we start with the wrptr same as what it was when we went into overflow.
    // Write one more packet to check that write is still functional after coming out of overflow.
    int pktSize = get_packet_size();
    c_stim_ep <: (int) GEN_RAMP;
    c_stim_ep <: (int) pktSize;

    // Now read everything till read returns underflow. Check that all the data got read by ensuring
    // that the last word read is the same as the last word written.
    unsigned out_underflow_val;
    GET_SHARED_GLOBAL(out_underflow_val, outUnderflow);
    while(out_underflow_val != 1)
    {
        c_stim_au <: (int)CHECK_RAMP;
        c_stim_au <: (int)rampCheck;
        rampCheck += (NUM_USB_CHAN_OUT); // Note, AudioHub receives in NUM_USB_CHAN_OUT sized blocks

        time += STIM_DELAY;
        t when timerafter(time) :> void;

        // Wait sometime before reading outUnderflow
        GET_SHARED_GLOBAL(out_underflow_val, outUnderflow);
    }
    assert(rampCheck == total_size_written && msg("Number of samples read not same as number of samples written"));

    printstr("PASS\n");
    c_stim_au <: -1;
    exit(0);

    return 0;
}

int Fake_XUA_AudioHub(chanend c_aud, chanend c_stim)
{
    int readBuffNo = 0;
    unsigned underflowWord = UNDERFLOW_WORD;
    int checkType;
    int checkData;

    while(1)
    {
        c_stim :> checkType;

        if(checkType == -1)
            return 0;
        c_stim :> checkData;

        unsigned command = DoSampleTransfer(c_aud, readBuffNo, underflowWord);

        CheckBuffer(samplesOut, NUM_USB_CHAN_OUT, checkType, checkData);
    }

    return 0;
}

int g_outEpReady = 0;
unsafe
{
    volatile int * unsafe gp_outEpReady = &g_outEpReady;
    volatile unsigned * unsafe gp_pktBuffer;
    volatile unsigned * unsafe gp_pktBuffer0;
}


/* This will be called by decouple */
XUD_Result_t XUD_SetReady_OutPtr(XUD_ep ep, unsigned addr)
{
    assert(g_outEpReady == 0);

    g_outEpReady = 1;

    unsafe
    {
        gp_pktBuffer = (unsigned * unsafe) addr;
    }

    return XUD_RES_OKAY;
}

extern unsigned outAudioBuff[];

int Fake_XUA_Buffer_Ep(chanend c_stim)
{
    int32_t pktLength;
    timer t;
    uint32_t time;
    t :> time;
    GenType_t genType;
    int rampVal = 0;
    size_t i = 0;
    unsigned * unsafe p_outAudioBuff;

    unsafe
    {
        p_outAudioBuff = &outAudioBuff[0];
    }

    xc_ptr aud_from_host_buffer = 0;
    SET_SHARED_GLOBAL(g_aud_from_host_flag, 1);

    while(!aud_from_host_buffer)
        GET_SHARED_GLOBAL(aud_from_host_buffer, g_aud_from_host_buffer);

    while(1)
    {
        c_stim :> genType;
        c_stim :> pktLength; // Packet length in bytes

        /* Only one command currenly supported */
        assert(genType == GEN_RAMP);

        /* Wait for decouple to mark EP as ready */
        unsafe{
            while(!*gp_outEpReady);
            *gp_outEpReady = 0;
        }

        i++;
        GET_SHARED_GLOBAL(aud_from_host_buffer, g_aud_from_host_buffer);
        //debug_printf("BUFFER_EP pkt %d; writing length %d to %d\n", i, pktLength, aud_from_host_buffer);
        assert(aud_from_host_buffer == gp_pktBuffer-1);
        write_via_xc_ptr(aud_from_host_buffer, pktLength); // aud_from_host_buffer is gp_pktBuffer--

        /* Populate buffer with data - emulating XUD */
        for(int i = 0; i < pktLength/sizeof(int); i++)
        unsafe
        {
            *(gp_pktBuffer++) = rampVal++;
        }

        /* Wait some time emulating delay before receiving packet */
        t when timerafter(time + 100) :> void;

        /* Sync with decouple thread */
        SET_SHARED_GLOBAL(g_aud_from_host_flag, 1);
    }

    return 0;
}

/**
 * Test: Buffer overflow and recovery across different states.
 *
 * Sequence:
 * - Write into an empty buffer till it goes into overflow.
 * - Then read from the buffer till it comes out of overflow. (This checks overflow recovery in the empty -> overflow case)
 * - Write into the buffer again till it goes into overflow.
 * - Then read from the buffer till it comes out of overflow. (This checks overflow recovery in the not-empty -> overflow case)
 * - Write again to ensure write is still functional after coming out of overflow.
 * - Read everything till read returns underflow. Check that all the data that was written gets read.
 *
 * empty -> overflow -> not overflow -> overflow -> not overflow -> empty
 *
 * The condition for overflow is different depending on whether the rdptr is at start or not. Hence
 * this test checks going into overflow from an empty (rdptr = start) and non-empty buffer (rdptr != start) state.
 *
 */
int main(void)
{
    chan c_stim_ep;
    chan c_stim_au;
    chan c_out;

    par
    {
        on tile[0]:
        {
            par
            {
                Fake_XUA_Buffer_Ep(c_stim_ep);
                XUA_Buffer_Decouple(c_out);
                Fake_XUA_AudioHub(c_out, c_stim_au);
                stim(c_stim_ep, c_stim_au);
            }
        }
    }

    return 0;
}

// Copyright 2023 XMOS LIMITED.
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

#define SAMPLE(frame_count, channel_num) (((frame_count) << 8) | ((channel_num) & 0xFF))
#define SAMPLE_FRAME_NUM(test_word) ((test_word) >> 8)
#define SAMPLE_CHANNEL_NUM(test_word) ((test_word) & 0xFF)
/* From xua_audiohub.xc */
extern unsigned samplesOut[NUM_USB_CHAN_OUT];
extern unsigned samplesIn[2][NUM_USB_CHAN_IN];
#include "xua_audiohub_st.h"

#define TEST_SAMPLE_RATE_HZ            (48000*4)
#define SAMPLE_PERIOD_NS               (1000000000/TEST_SAMPLE_RATE_HZ)
#define USB_FRAME_RATE_HZ              (8000) //HS
#define TEST_SAMPLE_SIZE_BYTES         (4)
#define PACKET_SAMPLES_PER_CHAN        (TEST_SAMPLE_RATE_HZ/USB_FRAME_RATE_HZ)
#define TEST_PACKET_SIZE_NOMINAL_BYTES (PACKET_SAMPLES_PER_CHAN * TEST_SAMPLE_SIZE_BYTES) * (NUM_USB_CHAN_OUT)

#define UNDERFLOW_WORD                 (0xBADDA55)
#define UNDERFLOW_PACKETS              (5) //TODO this is currently high and changes based on SR!


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
//TODO check this period
#define STIM_AUDIO_DELAY (10000)

int stim(chanend c_stim_ep, chanend c_stim_au)
{
    timer t;
    unsigned time;
    t :> time;
    int rampGen = 0;
    int rampCheck = 0;
    int underflowPackets = UNDERFLOW_PACKETS;

    /* Expect underflow */
    while(underflowPackets > 0)
    {
        c_stim_au <: (int)CHECK_STATIC;
        c_stim_au <: (int)UNDERFLOW_WORD;

        time += STIM_AUDIO_DELAY/2;
        t when timerafter(time) :> void;

        c_stim_ep <: (int) GEN_RAMP;
        c_stim_ep <: (int) rampGen;
        rampGen += (TEST_PACKET_SIZE_NOMINAL_BYTES/4);

        time += STIM_AUDIO_DELAY/2;
        t when timerafter(time) :> void;

        underflowPackets--;
    }

    time += STIM_AUDIO_DELAY;
    t when timerafter(time) :> void;

    /* We have given the buffering system "underflowPackets" which is underflowPackets * PACKET_SAMPLES_PER_CHAN * NUM_USB_CHAN_OUT samples */
    /* Drain this many samples then expect underflow */
    for(int i = 0; i < (UNDERFLOW_PACKETS * PACKET_SAMPLES_PER_CHAN); i++)
    {
        c_stim_au <: (int)CHECK_RAMP;
        c_stim_au <: (int)rampCheck;
        rampCheck += (NUM_USB_CHAN_OUT); // Note, AudioHub receives in NUM_USB_CHAN_OUT sized blocks

        time += STIM_AUDIO_DELAY;
        t when timerafter(time) :> void;
    }

    c_stim_au <: (int)CHECK_STATIC;
    c_stim_au <: (int)UNDERFLOW_WORD;

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

extern unsigned g_aud_from_host_flag;

int g_outEpReady = 0;
unsafe
{
    volatile int * unsafe gp_outEpReady = &g_outEpReady;
    volatile unsigned * unsafe gp_pktBuffer;
    volatile unsigned * unsafe gp_pktBuffer0;
}


/* This will be called by decouple */
inline int XUD_SetReady_OutPtr(XUD_ep ep, unsigned addr)
{
    assert(g_outEpReady == 0);

    g_outEpReady = 1;

    unsafe
    {
        gp_pktBuffer = (unsigned * unsafe) addr;
    }
}

extern unsigned outAudioBuff[];

int Fake_XUA_Buffer_Ep(chanend c_stim)
{
    int32_t length = TEST_PACKET_SIZE_NOMINAL_BYTES;
    timer t;
    uint32_t time;
    t :> time;
    GenType_t genType;
    int ramp = 1;
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
        /* Wait for decouple to mark EP as ready */
        unsafe{
            while(!*gp_outEpReady);
            *gp_outEpReady = 0;
        }

        i++;
        GET_SHARED_GLOBAL(aud_from_host_buffer, g_aud_from_host_buffer);
        debug_printf("BUFFER_EP pkt %d; writing length %d to %d\n", i, length, aud_from_host_buffer);
        write_via_xc_ptr(aud_from_host_buffer, length); // aud_from_host_buffer is gp_pktBuffer--

        assert(aud_from_host_buffer == gp_pktBuffer-1);

        c_stim :> genType;
        c_stim :> ramp;

        assert(genType == GEN_RAMP);

        /* Populate buffer with data - emulating XUD */
        for(int i = 0; i < length/sizeof(int); i++)
        unsafe
        {
            *(gp_pktBuffer++) = ramp++;
        }

        /* Wait some time emulating delay before receiving packet */
        t when timerafter(time + 100) :> void;

        /* Sync with decouple thread */
        SET_SHARED_GLOBAL(g_aud_from_host_flag, 1);
    }

    return 0;
}


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

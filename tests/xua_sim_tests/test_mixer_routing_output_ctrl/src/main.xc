// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* This tests checks the parsing of control requests to endpoint 0 cause the correct changes in mixer output routing */

#include <stdint.h>
#include <stddef.h>
#include "platform.h"
#include "xua.h"
#include "debug_print.h"
#include "assert.h"
#include "xud.h"
#include "usbaudio20.h"
#include "random.h"

#ifndef TEST_ITERATIONS
#define TEST_ITERATIONS (100)
#endif

#include "./../test_mixer_routing_output/src/mixer_test_shared.h"

/* Device channel mapping */
extern unsigned char channelMapAud[NUM_USB_CHAN_OUT];
extern unsigned char channelMapUsb[NUM_USB_CHAN_IN];

/* From xua_ep0_uacreqs.xc */
int AudioClassRequests_2(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t &sp, chanend ?c_audioControl, chanend ?c_mix_ctl, chanend ?c_clk_ctl);

/* From xua_endpoint0.c */
void InitLocalMixerState();

int g_src = 0;

/* Override func in lib_xud/src/user/client/XUD_EpFunctions.c for testing purposes */
XUD_Result_t XUD_GetBuffer(XUD_ep ep_out, unsigned char buffer[], REFERENCE_PARAM(unsigned, length))
{
    buffer[0] = g_src;
    return XUD_RES_OKAY;
}

XUD_Result_t XUD_DoSetRequestStatus(XUD_ep ep_in)
{
    return XUD_RES_OKAY;
}

XUD_Result_t XUD_SetBuffer_EpMax(XUD_ep ep_in, unsigned char buffer[], unsigned datalength, unsigned epMax)
{
    assert(g_src == buffer[0]);
    assert(datalength == 1);
    return XUD_RES_OKAY;
}

unsafe
{
    extern int volatile * const unsafe samples_to_device_map;
    extern int volatile * const unsafe samples_to_host_map;
}

void Fake_Endpoint0(chanend c_mix_ctl)
{
    XUD_ep ep0_out;  /* Never initialised but not used */
    XUD_ep ep0_in;   /* Never initialised but not used */
    unsigned unitIds[] = {ID_XU_OUT, ID_XU_IN};
    USB_SetupPacket_t sp;

    random_generator_t rg = random_create_generator_from_seed(TEST_SEED);

    InitLocalMixerState();

    sp.bmRequestType.Type = USB_BM_REQTYPE_TYPE_CLASS;         // Note, parsing of this won't be tested since we call AudioClassRequests directly
    sp.bmRequestType.Recipient = USB_BM_REQTYPE_RECIP_INTER;   // Note, parsing of this won't be tested since we call AudioClassRequests directly

    for(int testIter = 0; testIter < TEST_ITERATIONS; testIter++)
    {
        int unitId = unitIds[random_get_random_number(rg) % (sizeof(unitIds)/sizeof(unitIds[0]))];
        unsigned dst = random_get_random_number(rg);

        /* Note, we don't currently support a mix input derived from another mix
        * This is not trivial to test since the current mixer implementation only allows for one
        * config update per "trigger"
        */
        int src = random_get_random_number(rg) % NUM_USB_CHAN_IN + NUM_USB_CHAN_OUT;

        switch(unitId)
        {
            case ID_XU_OUT:
                dst %= CHANNEL_MAP_AUD_SIZE;
                debug_printf("Mapping output to AudioIF: %d", dst);
                debug_printf(" from %d", src);
                PrintSourceString(src);
                debug_printf("\n");
                break;

            case ID_XU_IN:
                dst %= CHANNEL_MAP_USB_SIZE;
                debug_printf("Mapping output to Host : %d", dst);
                debug_printf(" from %d", src);
                PrintSourceString(src);
                debug_printf("\n");
                break;

            default:
                printstr("ERROR: Bad cmd in stim(): ");
                printintln(unitId);
              break;
        }

        /* Create Control request data for routing change */
        sp.bmRequestType.Direction = USB_BM_REQTYPE_DIRECTION_H2D;
        sp.bRequest = CUR;
        sp.wValue = dst & 0xff;
        sp.wIndex = (unitId << 8);
        sp.wLength = 1;

        g_src = src; /* This will get picked up by out implementation of XUD_GetBuffer */

        /* Call the function used by Endpoint0() to parse the control data and update the mixer output routing */
        AudioClassRequests_2(ep0_out, ep0_in, sp, null, c_mix_ctl, null);

        /* Note, there is a race risk here. This could be resolved by adding a handshake to UpdateMixerOutputRouting() etc */

        /* Now check the mixer setting have been modified as expected. To do this we inspect "internal"
         * mixer and endpoint 0 state.
         *
         * Going forward we might wish to enhance the mixer API such that it can be tested as a black box.
         * This would require the addition of "GET" API over it's ctrl channel
         */
        switch(unitId)
        {
            case ID_XU_OUT:
                assert(g_src == channelMapAud[dst]);
                unsafe
                {
                    assert(g_src == samples_to_device_map[dst]);
                }
                break;

            case ID_XU_IN:
                assert(g_src == channelMapUsb[dst]);
                unsafe
                {
                    assert(g_src == samples_to_host_map[dst]);
                }
                break;

            default:
                assert(0);
                break;
        }

        /* Test read back. Note, the checking is our overridden implementation of XUD_SetBuffer_EpMax*/
        sp.bmRequestType.Direction = USB_BM_REQTYPE_DIRECTION_D2H;
        AudioClassRequests_2(ep0_out, ep0_in, sp, null, c_mix_ctl, null);
    }

    printstrln("PASS");
    exit(0);
}

void Fake_XUA_AudioHub_CtrlTest(chanend c_mix_aud)
{
    int readBuffNo = 0;
    unsigned underflowWord = 0;

    /* Continually send/receive samples to/from mixer, no checking of samples since this is purely a control test */
    while(1)
    {
        unsigned command = DoSampleTransfer(c_mix_aud, readBuffNo, underflowWord);
    }
}

void Fake_XUA_Buffer_Decouple_CtrlTest(chanend c_dec_mix)
{
    unsigned samplesIn[NUM_USB_CHAN_IN];
    unsigned underflowSample;

    /* Continually send/receive samples to/from mixer, no checking of samples since this is purely a control test */
    while(1)
    {
        select
        {
            case inuint_byref(c_dec_mix, underflowSample):

                for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
                {
                    outuint(c_dec_mix, 0);
                }

                for(int i = 0; i < NUM_USB_CHAN_IN; i++)
                {
                    samplesIn[i] = inuint(c_dec_mix);
                }

                break;
        }
    }
}

int main()
{
    chan c_dec_mix;
    chan c_mix_aud;
    chan c_mix_ctl;

    par
    {
        /* We need "fake" versions of the AudioHub and Decouple to keep the mixer running and taking updates via
         * it's control channel */
        Fake_XUA_Buffer_Decouple_CtrlTest(c_dec_mix);
        Fake_XUA_AudioHub_CtrlTest(c_mix_aud);

        /* Mixer from lib_xua */
        mixer(c_dec_mix, c_mix_aud, c_mix_ctl);

        Fake_Endpoint0(c_mix_ctl);
    }

    /* TODO to hit this we need to fully close down i.e. kill mixer */
    return 0;
}


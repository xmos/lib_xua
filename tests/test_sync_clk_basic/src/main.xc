// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* Simple test to ensure reference clock to CS2100 device continues when SOF clock not available
 * Note, this test uses "nice" numbers (i.e. MISSIG_SOFS %8 == 0) and therefore doesn't check
 * for a graceful change over from internal to SOF clock
 */

#include "platform.h"
#include "xua.h"
#include "print.h"
#include "xud.h"

#define EP_COUNT_IN        (3)
#define EP_COUNT_OUT       (3)

out port p_pll_ref         = XS1_PORT_1A;
in port p_off_mclk         = XS1_PORT_1M;
in port p_pll_loop         = XS1_PORT_1B; /* Note, this is externally looped back using the loopback plugin */

/* Purely for debug/viewing on VCD */
out port p_test0           = XS1_PORT_1C;
out port p_test1           = XS1_PORT_1D;

#ifndef BUS_SPEED
#error BUS_SPEED should be defined
#endif

/* To speed this test up we divide all delays by 10. This is also the case for the delays in the clock generation code */
#if(BUS_SPEED == 2) // XUD_SPEED_HS
    #define SOF_PERIOD_TICKS   (12500/10)
    #define SOF_DIVIDE         (1)
#else
    #define SOF_PERIOD_TICKS   ((12500*8)/10)
    #define SOF_DIVIDE         (8)
#endif

#ifndef MISSING_SOF_PERIOD
/* By default skip a whole number of SOF periods (easy case)
 * Note, app_test_sync_plugin/Makefiles sets this to something more nasty */
#define MISSING_SOF_PERIOD       (8 * SOF_PERIOD_TICKS)
#endif

void exit(int);

void delay(unsigned d)
{
    timer t;
    unsigned time;
    t :> time;
    t when timerafter(time + d) :> int x;
}

/* From lib_xud */
void SetupEndpoints(chanend c_ep_out[], int noEpOut, chanend c_ep_in[], int noEpIn, XUD_EpType epTypeTableOut[], XUD_EpType epTypeTableIn[]);

void AudioHwInit()
{
    return;
}

void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    return;
}

void driveSofs(chanend c_sof, int count)
{
    static int frame = 0;

    /* For HS frame should only increment every 8 SOFs, but this is a "dont care" */
    for(int i = 0; i < count; i++)
    {
        outuint(c_sof, frame++);
        delay(SOF_PERIOD_TICKS);
    }
}

void fake_xud(chanend c_out[], chanend c_in[], chanend c_sof)
{
    timer t;
    unsigned time;


    /* Makes traces a bit nicer to look at */
    t :> time;
    t when timerafter(SOF_PERIOD_TICKS * 2) :> int x;

    p_test0 <: 1;

    /* Endpoint type tables */
    XUD_EpType epTypeTableOut[EP_COUNT_OUT] = {XUD_EPTYPE_CTL, XUD_EPTYPE_ISO, XUD_EPTYPE_DIS};
    XUD_EpType epTypeTableIn[EP_COUNT_IN] =   {XUD_EPTYPE_CTL, XUD_EPTYPE_ISO, XUD_EPTYPE_ISO};

    SetupEndpoints(c_out, EP_COUNT_OUT, c_in, EP_COUNT_IN, epTypeTableOut, epTypeTableIn);

    driveSofs(c_sof, 32/SOF_DIVIDE);

    p_test0 <: 0;

    /* Sim missing SOFs */
    delay(MISSING_SOF_PERIOD);

    p_test0 <: 1;

    driveSofs(c_sof, 16/SOF_DIVIDE);

    p_test0 <: 0;

    delay(MISSING_SOF_PERIOD);

    p_test0 <: 1;

    driveSofs(c_sof, 16/SOF_DIVIDE);

    p_test0 <: 0;

}

extern XUD_BusSpeed_t g_curUsbSpeed;

#define MARGIN (1500/10)
#define EXPECTED_PERIOD (100000/10)

void checker()
{
    timer t;
    unsigned t0, t1;
    unsigned x = 0;
    int fail = 0;

    p_test1 <: 1;

    p_pll_loop when pinseq(1) :> x;
    p_pll_loop when pinseq(0) :> x;
    p_pll_loop when pinseq(1) :> x;

    for(int i = 0; i < 12; i++)
    {
        p_pll_loop when pinsneq(x) :>  x;
        t :> t0;
        p_pll_loop when pinsneq(x) :>  x;
        t :> t1;

        int period = t1-t0;

        /* Check the period of the reference clock we are generating */
        if(period > (EXPECTED_PERIOD + MARGIN))
        {
            printstr("Period too long: ");
            printintln(period);
            fail = 1;
        }
        else if(period < (EXPECTED_PERIOD - MARGIN))
        {
            printstr("Period too short: ");
            printintln(period);
            fail = 1;
        }
    }
    if(!fail)
        printstrln("PASS");

    p_test1 <: 0;

    exit(0);
}


int main()
{
    chan c_out[EP_COUNT_OUT];
    chan c_in[EP_COUNT_IN];
    chan c_sof;
    chan c_aud_ctl;

    interface pll_ref_if i_pll_ref;

    par
    {
        PllRefPinTask(i_pll_ref, p_pll_ref);

        {
            g_curUsbSpeed = BUS_SPEED;

            XUA_Buffer_Ep(c_out[1],      /* USB Audio Out*/
                c_in[1],                 /* USB Audio In */
                c_sof, c_aud_ctl, p_off_mclk, i_pll_ref
            );
        }

        fake_xud(c_out, c_in, c_sof);

        checker();
    }
}

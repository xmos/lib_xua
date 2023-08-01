// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <print.h>
#include <platform.h>
#include "xua.h"
#include "xassert.h"
#include <stdio.h>
#if (XUA_USE_APP_PLL)

/*
 * Functions for interacting with the secondary/application PLL
 */

#ifndef __XS3A__
    #error App PLL not included in device
#endif

/*
 * App PLL settings used for syncing to external clocks
 */

// Found solution: IN 24.000MHz, OUT 24.575758MHz, VCO 3244.00MHz, RD  3, FD  405.500 (m =   1, n =   2), OD  3, FOD   11, ERR -9.864ppm
#define APP_PLL_CTL_SYNC_24M         (0x09019402)
#define APP_PLL_DIV_SYNC_24M         (0x8000000A)
#define APP_PLL_FRAC_SYNC_24M        (0x80000001)

// Fout = (Fin/3)*divider/(2*2*3*11) = (fin/396) * divider = (24/396) * divider. = 2/33 * divider.
// Total freq tune range = ((406/405) - 1) * 1000000 ppm = 2469ppm.
// Step size: 2469/2^20 ~= 2.4ppb.
// Setting of 0 (0x000000) => Divide of 405. Output freq = (2/33) * 405 = 24.545MHz. This is -1244ppm from ideal 24.576MHz.
// Setting of 1 (0xFFFFFF) => Divide of 406. Output freq = (2/33) * 406 = 24.606MHz. This is +1223ppm from ideal 24.576MHz.

// To achieve frequency f, Fraction Setting = ((33/2)*f) - 405
// So to achieve 24.576MHz, Fraction Setting = (16.5*24.576) - 405 = 0.504
// Numerical input = round((Fraction setting * 2^20) = 0.504 * 1048576 = 528482 = 0x81062.

// Found solution: IN 24.000MHz, OUT 22.579365MHz, VCO 3251.43MHz, RD  3, FD  406.429 (m =   3, n =   7), OD  6, FOD    6, ERR 7.311ppm
#define APP_PLL_CTL_SYNC_22M        (0x0A819502)
#define APP_PLL_DIV_SYNC_22M        (0x80000005)
#define APP_PLL_FRAC_SYNC_22M       (0x80000206)

// Fout = (Fin/3)*divider/(2*2*3*11) = (fin/396) * divider = (24/396) * divider. = 2/33 * divider.
// Total freq tune range = ((406/405) - 1) * 1000000 ppm = 2469ppm.
// Step size: 2469/2^20 ~= 2.4ppb.
// Setting of 0 (0x000000) => Divide of 405. Output freq = (2/33) * 405 = 24.545MHz. This is -1244ppm from ideal 24.576MHz.
// Setting of 1 (0xFFFFFF) => Divide of 406. Output freq = (2/33) * 406 = 24.606MHz. This is +1223ppm from ideal 24.576MHz.

// To achieve frequency f, Fraction Setting = ((33/2)*f) - 405
// So to achieve 24.576MHz, Fraction Setting = (16.5*24.576) - 405 = 0.504
// Numerical input = round((Fraction setting * 2^20) = 0.504 * 1048576 = 528482 = 0x81062.

/*
 * App PLL settings used for low jitter fixed local clocks
 */

//Found solution: IN 24.000MHz, OUT 49.151786MHz, VCO 3145.71MHz, RD 1, FD 131.071 (m = 1, n = 14), OD 8, FOD 2, ERR -4.36ppm
// Measure: 100Hz-40kHz: ~7ps
// 100Hz-1MHz: 70ps.
// 100Hz high pass: 118ps.
#define APP_PLL_CTL_FIXED_49M       (0x0B808200)
#define APP_PLL_DIV_FIXED_49M       (0x80000001)
#define APP_PLL_FRAC_FIXED_49M      (0x8000000D)

//Found solution: IN 24.000MHz, OUT 45.157895MHz, VCO 2709.47MHz, RD 1, FD 112.895 (m = 17, n = 19), OD 5, FOD 3, ERR -11.19ppm
// Measure: 100Hz-40kHz: 6.5ps
// 100Hz-1MHz: 67ps.
// 100Hz high pass: 215ps.
#define APP_PLL_CTL_FIXED_45M       (0x0A006F00)
#define APP_PLL_DIV_FIXED_45M       (0x80000002)
#define APP_PLL_FRAC_FIXED_45M      (0x80001012)

// Found solution: IN 24.000MHz, OUT 24.576000MHz, VCO 2457.60MHz, RD 1, FD 102.400 (m = 2, n = 5), OD 5, FOD 5, ERR 0.0ppm
// Measure: 100Hz-40kHz: ~8ps
// 100Hz-1MHz: 63ps.
// 100Hz high pass: 127ps.
#define APP_PLL_CTL_FIXED_24M       (0x0A006500)
#define APP_PLL_DIV_FIXED_24M       (0x80000004)
#define APP_PLL_FRAC_FIXED_24M      (0x80000104)

// Found solution: IN 24.000MHz, OUT 22.579186MHz, VCO 3522.35MHz, RD 1, FD 146.765 (m = 13, n = 17), OD 3, FOD 13, ERR -0.641ppm
// Measure: 100Hz-40kHz: 7ps
// 100Hz-1MHz: 67ps.
// 100Hz high pass: 260ps.
#define APP_PLL_CTL_FIXED_22M       (0x09009100)
#define APP_PLL_DIV_FIXED_22M       (0x8000000C)
#define APP_PLL_FRAC_FIXED_22M      (0x80000C10)

#define APP_PLL_CTL_FIXED_12M       (0x0A006500)
#define APP_PLL_DIV_FIXED_12M       (0x80000009)
#define APP_PLL_FRAC_FIXED_12M      (0x80000104)

#define APP_PLL_CTL_FIXED_11M       (0x09009100)
#define APP_PLL_DIV_FIXED_11M       (0x80000009)
#define APP_PLL_FRAC_FIXED_11M      (0x80000C10)

#define APP_PLL_CTL_ENABLE          (1 << 27)
#define APP_PLL_CLK_OUTPUT_ENABLE   (1 << 16)

static void set_app_pll_init(tileref tile, int app_pll_ctrl)
{
    // Disable the PLL
    write_node_config_reg(tile, XS1_SSWITCH_SS_APP_PLL_CTL_NUM, (app_pll_ctrl & ~APP_PLL_CTL_ENABLE));

    // Enable the PLL to invoke a reset on the appPLL.
    write_node_config_reg(tile, XS1_SSWITCH_SS_APP_PLL_CTL_NUM, app_pll_ctrl);

    // Must write the CTL register twice so that the F and R divider values are captured using a running clock.
    write_node_config_reg(tile, XS1_SSWITCH_SS_APP_PLL_CTL_NUM, app_pll_ctrl);

    // Now disable and re-enable the PLL so we get the full 5us reset time with the correct F and R values.
    write_node_config_reg(tile, XS1_SSWITCH_SS_APP_PLL_CTL_NUM, (app_pll_ctrl & 0xF7FFFFFF));
    write_node_config_reg(tile, XS1_SSWITCH_SS_APP_PLL_CTL_NUM, app_pll_ctrl);

    // Wait for PLL to lock.
    delay_microseconds(500);
}



void AppPllEnable(tileref tile, int clkFreq_hz)
{
    unsigned app_pll_ctrl, app_pll_div, app_pll_frac;

    /* Decide on App PLL settings */
    if(XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
    {
        switch(clkFreq_hz)
        {
            case 44100 * 512:
                app_pll_ctrl = APP_PLL_CTL_SYNC_22M;
                app_pll_div = APP_PLL_DIV_SYNC_22M;
                app_pll_frac = APP_PLL_FRAC_SYNC_22M;
                break;

            case 48000 * 512:
                app_pll_ctrl = APP_PLL_CTL_SYNC_24M;
                app_pll_div = APP_PLL_DIV_SYNC_24M;
                app_pll_frac = APP_PLL_FRAC_SYNC_24M;
                break;

            default:
                assert(0);
                break;
        }
    }
    else
    {
        switch(clkFreq_hz)
        {
            case 44100 * 256:
                app_pll_ctrl = APP_PLL_CTL_FIXED_11M;
                app_pll_div = APP_PLL_DIV_FIXED_11M;
                app_pll_frac = APP_PLL_FRAC_FIXED_11M;
                break;

             case 48000 * 256:
                app_pll_ctrl = APP_PLL_CTL_FIXED_12M;
                app_pll_div = APP_PLL_DIV_FIXED_12M;
                app_pll_frac = APP_PLL_FRAC_FIXED_12M;
                break;

            case 44100 * 512:
                app_pll_ctrl = APP_PLL_CTL_FIXED_22M;
                app_pll_div = APP_PLL_DIV_FIXED_22M;
                app_pll_frac = APP_PLL_FRAC_FIXED_22M;
                break;

            case 48000 * 512:
                app_pll_ctrl = APP_PLL_CTL_FIXED_24M;
                app_pll_div = APP_PLL_DIV_FIXED_24M;
                app_pll_frac = APP_PLL_FRAC_FIXED_24M;
                break;

            case 44100 * 1024:
                app_pll_ctrl = APP_PLL_CTL_FIXED_45M;
                app_pll_div = APP_PLL_DIV_FIXED_45M;
                app_pll_frac = APP_PLL_FRAC_FIXED_45M;
                break;

            case 48000 * 1024:
                app_pll_ctrl = APP_PLL_CTL_FIXED_49M;
                app_pll_div = APP_PLL_DIV_FIXED_49M;
                app_pll_frac = APP_PLL_FRAC_FIXED_49M;
                break;

            default:
                assert(0);
                break;
        }
    }

    // Initialise the AppPLL and get it running.
    set_app_pll_init(tile, app_pll_ctrl);

    // Write the fractional-n register, note, the top bit is set to enable the frac-n block.
    write_node_config_reg(tile, XS1_SSWITCH_SS_APP_PLL_FRAC_N_DIVIDER_NUM, app_pll_frac);

    // And then write the clock divider register to enable the output
    write_node_config_reg(tile, XS1_SSWITCH_SS_APP_CLK_DIVIDER_NUM, app_pll_div);

    // Wait for PLL output frequency to stabilise due to fractional divider enable
    delay_microseconds(100);
}

void SoftPllInit(int clkFreq_hz, struct SoftPllState &pllState)
{
    switch(clkFreq_hz)
    {
         case 44100 * 512:
            pllState.expectedClkMod = 29184;
            pllState.initialSetting = 0x6CF42;
            break;

        case 48000 * 512:
            pllState.expectedClkMod = 49152;
            pllState.initialSetting = 0x81062;
            break;

         default:
            assert(0);
            break;
    }
    pllState.firstUpdate = 1;

    pllState.ds_in = pllState.initialSetting;
    pllState.ds_fb = 0;
    pllState.ds_x1 = 0;
    pllState.ds_x2 = 0;
}

int SoftPllUpdate(tileref tile, unsigned short mclk_pt, unsigned short mclk_pt_last, struct SoftPllState &pllState)
{
    static int int_error = 0;

    unsigned expectedClksMod = pllState.expectedClkMod;
    unsigned initialSetting = pllState.initialSetting;

    // TODO These values need revisiting
    const int Kp = 0;
    const int Ki = 32;

    int newsetting;
    int abs_error, error_p, error_i;
    unsigned short expectedPt;

    int set = -1;
    int diff;

    // expectedClkMod is the value of the port counter that we expect given the desired MCLK in the 10ms time period we are running at.
    expectedPt = mclk_pt_last + expectedClksMod;

    // Handle wrapping
    if (porttimeafter(mclk_pt, expectedPt))
    {
        diff = -(short)(expectedPt - mclk_pt);
    }
    else
    {
        diff = (short)(mclk_pt - expectedPt);
    }

    // TODO Add a bounds checker on diff to make sure it's roughly where we expect.
    // If it isn't we should ignore it as it's either a glitch or from clock start/stop.

    // Absolute error for last measurement cycle. If diff is positive, port timer was beyond where it should have been, so MCLK was too fast. So this needs to describe a negative error.
    abs_error = -diff;
    int_error = int_error + abs_error; // Integral error.

    error_p = (Kp * abs_error);
    error_i = (Ki * int_error);

    newsetting = (error_p + error_i);

    // Only output new frequency tune value if different to the previous setting
    if (newsetting != pllState.setting)
    {
        set = (initialSetting + newsetting); // init_set is the value to feed into the NCO to give the "expected" frequency (24.576 or 22.5792). Not required but will make lock faster.
        if (set < 0)
            set = 0;
        else if (set > 0xFFFFF)
            set = 0xFFFFF;
    }

    pllState.setting = newsetting;

    // Return the setting to the NCO thread. -1 means no update
    return set;
}

#if (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
[[distributable]]
#endif
void XUA_SoftPll(tileref tile, server interface SoftPll_if i_softPll, chanend c_update)
{
    unsigned ds_out;
    timer tmr;
    int time;
    unsigned mclk_pt;
    unsigned short mclk_pt_last;
    struct SoftPllState pllState;
    int running = 0;
    int firstUpdate = 1;

    tmr :> time;

    while(1)
    {
        select
        {
            /* Interface used for basic frequency setting such that it can be distributed
             * when the update code is not required */
            case i_softPll.init(int mclk_hz):
                AppPllEnable(tile, mclk_hz);
                SoftPllInit(mclk_hz, pllState);
                running = 1;
                firstUpdate = 1;
                break;

#if (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
        }
    }
}
#else
            /* Channel used for update such that other side is not blocked */
            /* TODO add CT handshake before opening route */
            case inuint_byref(c_update, mclk_pt):
                inct(c_update);

                if(firstUpdate)
                {
                    firstUpdate = 0;
                }
                else
                {
                    int setting = SoftPllUpdate(tile, (unsigned short) mclk_pt, mclk_pt_last, pllState);

                    if(setting != -1)
                    {
                        pllState.ds_in = setting;
                        pllState.ds_in = pllState.ds_in & 0x000FFFFF; // Ensure input is limited to 20 bits
                    }
                }

                mclk_pt_last = (unsigned short) mclk_pt;
                break;

            default :
                break;

     }

        // Second order, Single bit delta sigma - mod2
        pllState.ds_x1 += pllState.ds_in - pllState.ds_fb;
        pllState.ds_x2 += pllState.ds_x1 - pllState.ds_fb;
        if (pllState.ds_x2 < 0)
        {
            ds_out = 0;
            pllState.ds_fb = 0;
        }
        else
        {
            ds_out = 1;
            pllState.ds_fb = 1048575; //pow(2, 20)
        }

        // Now write the register.
        // We need to write the register at a specific period at a fast rate.
        // This period needs to be (div ref clk period (ns) * how many times we repeat same value)
        // In this case, div ref clk = 24/3 = 8MHz. So div ref clk period = 125ns.
        // So minimum period we could use is 125ns.
        // We use sw ref clk to time the register write. This uses 10ns clock ticks.
        // So this period should be a multiple of 10ns too.
        // So shortest period to meet these requirements is 250ns. This is still very fast though.
        // 1000ns is a great choice if we can.
        // 1500ns is a better choice.
        // 2250ns is the next choice.
        // The slower we write, our jitter goes up significantly.
        // Measuring rms jitter 100Hz-40kHz and TIE (TIE across 80ms of clock output).
        // 750ns  - jitter = 51ps,  tie = +-1.5ns.
        // 1500ns - jitter = 110ps, tie = +-2.5ns.
        // 2250ns - jitter = 162ps, tie = +-3.2ns. // Note to be measured for single bit DS.

        time += 150; // We write reg every 1500ns.
        tmr when timerafter(time) :> void;

        // ds_out = 1 => fraction of 1/1 = N+1. ds_out = 0 => fraction of 0/1 = N.
        if(running)
        {
            // Write the register. Because we are timing the reg writes accurately we do not need to use reg write with ack.
            // This saves a lot of time. Additionally, apparently we can shorten the time for this reg write by only setting up
            // the channel once and just doing a few instructions to do the write each time.
            // We can hard code this in assembler.

            write_node_config_reg_no_ack(tile, XS1_SSWITCH_SS_APP_PLL_FRAC_N_DIVIDER_NUM, (ds_out << 31));
        }
    }
}
#endif
#endif

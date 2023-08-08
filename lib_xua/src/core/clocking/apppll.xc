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

// Define the PLL settings to generate the required frequencies.
// All settings allow greater than +-1000ppm lock range.
// Comment out the following line for 2us update.

//#define FAST_FRAC_REG_WRITE

// OPTION 1 - 1us register update rate - Lowest jitter
// 10ps jitter 100Hz-40kHz. Low freq noise floor -100dBc

#ifdef FAST_FRAC_REG_WRITE

#define FRAC_REG_WRITE_DLY          (100)

// Found solution: IN 24.000MHz, OUT 22.578947MHz, VCO 3251.37MHz, RD  1, FD  135.474 (m =   9, n =  19), OD  6, FOD    6, ERR -11.189ppm
#define APP_PLL_CTL_SYNC_22M        (0x0A808600)
#define APP_PLL_DIV_SYNC_22M        (0x80000005)
#define APP_PLL_FRAC_SYNC_22M       (0x80000812)
#define APP_PLL_MOD_INIT_22M        (498283)

// Fout = Fin*divider/(2*2*6*6) = (fin/144) * divider = (24/144) * divider. = 1/6 * divider.
// To achieve frequency f, Fraction Setting = (6*f) - 135
// So to achieve 22.5792MHz, Fraction Setting = (6*22.5792) - 135 = 0.4752
// Numerical input = round((Fraction setting * 2^20) = 0.4752 * 1048576 = 498283

//Found solution: IN 24.000MHz, OUT 24.575758MHz, VCO 3538.91MHz, RD  1, FD  147.455 (m =   5, n =  11), OD  6, FOD    6, ERR -9.864ppm
#define APP_PLL_CTL_SYNC_24M        (0x0A809200)
#define APP_PLL_DIV_SYNC_24M        (0x80000005)
#define APP_PLL_FRAC_SYNC_24M       (0x8000040A)
#define APP_PLL_MOD_INIT_24M        (478151)

// Fout = Fin*divider/(2*2*6*6) = (fin/144) * divider = (24/144) * divider. = 1/6 * divider.
// To achieve frequency f, Fraction Setting = (6*f) - 147
// So to achieve 24.576MHz, Fraction Setting = (6*24.576) - 147 = 0.456
// Numerical input = round((Fraction setting * 2^20) = 0.456 * 1048576 = 478151

#else

// OPTION 2 - 2us register update rate - Higher jitter
// 50ps jitter 100Hz-40kHz. Low freq noise floor -93dBc

#define FRAC_REG_WRITE_DLY          (200)

//Found solution: IN 24.000MHz, OUT 22.579186MHz, VCO 3522.35MHz, RD  2, FD  293.529 (m =   9, n =  17), OD  3, FOD   13, ERR -0.641ppm
#define APP_PLL_CTL_SYNC_22M        (0x09012401)
#define APP_PLL_DIV_SYNC_22M        (0x8000000C)
#define APP_PLL_FRAC_SYNC_22M       (0x80000810)
#define APP_PLL_MOD_INIT_22M        (555326)

// Fout = (Fin/2)*divider/(2*2*3*13) = (fin/312) * divider = (24/312) * divider. = 1/13 * divider.
// To achieve frequency f, Fraction Setting = (13*f) - 293
// So to achieve 22.5792MHz, Fraction Setting = (13*22.5792) - 293 = 0.5296
// Numerical input = round((Fraction setting * 2^20) = 0.5296 * 1048576 = 555326

//Found solution: IN 24.000MHz, OUT 24.576125MHz, VCO 3342.35MHz, RD  2, FD  278.529 (m =   9, n =  17), OD  2, FOD   17, ERR 5.069ppm - Runs VCO out fractionally out of spec at 835MHz
#define APP_PLL_CTL_SYNC_24M        (0x08811501)
#define APP_PLL_DIV_SYNC_24M        (0x80000010)
#define APP_PLL_FRAC_SYNC_24M       (0x80000810)
#define APP_PLL_MOD_INIT_24M        (553648)

// Fout = (Fin/2)*divider/(2*2*2*17) = (fin/272) * divider = (24/272) * divider. = 3/34 * divider.
// To achieve frequency f, Fraction Setting = ((34/3)*f) - 278
// So to achieve 24.576MHz, Fraction Setting = ((34/3)*24.576) - 278 = 0.528
// Numerical input = round((Fraction setting * 2^20) = 0.528 * 1048576 = 553648
#endif

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
            pllState.expectedClkMod = 29184; // Count we expect on MCLK port timer at SW PLL check point. For 100Hz, 10ms.
            pllState.initialSetting = APP_PLL_MOD_INIT_22M;
            break;

        case 48000 * 512:
            pllState.expectedClkMod = 49152;
            pllState.initialSetting = APP_PLL_MOD_INIT_24M;
            break;

         default:
            assert(0);
            break;
    }
    pllState.firstUpdate = 1;

    pllState.ds_in = pllState.initialSetting;
    pllState.ds_x1 = 0;
    pllState.ds_x2 = 0;
    pllState.ds_x3 = 0;
}

int SoftPllUpdate(tileref tile, unsigned short mclk_pt, unsigned short mclk_pt_last, struct SoftPllState &pllState)
{
    static int int_error = 0;

    unsigned expectedClksMod = pllState.expectedClkMod;
    unsigned initialSetting = pllState.initialSetting;

    // TODO These values need revisiting/making fixed point
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
#if (XUA_SYNCMODE != XUA_SYNCMODE_ASYNC)
    unsigned frac_val;
    int ds_out;
    timer tmr;
    int time;
    unsigned mclk_pt;
    unsigned short mclk_pt_last;
    tmr :> time;
#endif
    struct SoftPllState pllState;
    int running = 0;
    int firstUpdate = 1;

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
                        // Limit input range for modulator stability.
                        if(pllState.ds_in > 980000)
                            pllState.ds_in = 980000;
                        if(pllState.ds_in < 60000)
                            pllState.ds_in = 60000;
                    }
                }

                mclk_pt_last = (unsigned short) mclk_pt;
                break;

            default :
                break;

        }

        // Third order, 9 level output delta sigma. 20 bit unsigned input.
        ds_out = ((pllState.ds_x3<<4) + (pllState.ds_x3<<1)) >> 13;
        if (ds_out > 8)
          ds_out = 8;
        if (ds_out < 0)
          ds_out = 0;
        pllState.ds_x3 += (pllState.ds_x2>>5) - (ds_out<<9) - (ds_out<<8);
        pllState.ds_x2 += (pllState.ds_x1>>5) - (ds_out<<14);
        pllState.ds_x1 += pllState.ds_in - (ds_out<<17);

        if (ds_out == 0)
            frac_val = 0x00000007; // 0/8
        else
            frac_val = ((ds_out - 1) << 8) | 0x80000007; // 1/8 to 8/8

        // Now write the register.
        // We need to write the register at a specific period at a fast rate.
        // This period needs to be (div ref clk period (ns) * how many times we repeat same value)
        // In this case, div ref clk = 24/3 = 8MHz. So div ref clk period = 125ns.
        // We're using fraction denominators of 8, so these repeat every 8*125ns = 1us.
        // So minimum period we could use is 1us and multiples thereof.
        // The slower we write, the higher our jitter will be.

        time += FRAC_REG_WRITE_DLY; // Time the reg write.
        tmr when timerafter(time) :> void;

        // Write the register. Because we are timing the reg writes accurately we do not need to use reg write with ack.
        // This saves a lot of time. Additionally, apparently we can shorten the time for this reg write by only setting up the channel once and just doing a few instructions to do the write each time.
        // We can hard code this in assembler.
        if(running)
        {
            write_node_config_reg_no_ack(tile, XS1_SSWITCH_SS_APP_PLL_FRAC_N_DIVIDER_NUM, frac_val);
        }
    }
}
#endif
#endif

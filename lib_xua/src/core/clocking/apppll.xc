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

#include "fractions_80_top.h"

// This can do +350ppm.
//Found solution: IN 24.000MHz, OUT 22.579211MHz, VCO 3070.77MHz, RD  2, FD  255.898 (m =  79, n =  88), OD  2, FOD   17, ERR 0.497ppm
#define APP_PLL_CTL_SYNC_22M        (0x0880FE01)
#define APP_PLL_DIV_SYNC_22M        (0x80000010)
#define APP_PLL_F_INDEX_SYNC_22M    (193)

//This can do +256ppm.
//Found solution: IN 24.000MHz, OUT 24.576007MHz, VCO 3538.95MHz, RD  2, FD  294.912 (m =  83, n =  91), OD  3, FOD   12, ERR 0.298ppm
#define APP_PLL_CTL_SYNC_24M        (0x09012501)
#define APP_PLL_DIV_SYNC_24M        (0x8000000B)
#define APP_PLL_F_INDEX_SYNC_24M    (220)

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

void AppPllGetSettings(int clkFreq_hz, struct PllSettings &pllSettings)
{
    pllSettings.firstUpdate = 1;

    switch(clkFreq_hz)
    {
         case 44100 * 512:
            pllSettings.fracIdx = APP_PLL_F_INDEX_SYNC_22M;
            pllSettings.adder = 29184;
            break;

        case 48000 * 512:
            pllSettings.fracIdx = APP_PLL_F_INDEX_SYNC_24M;
            pllSettings.adder = 49152;
            break;

         default:
            assert(0);
            break;
    }
}

//unsigned sw_pll_adder;
//unsigned app_pll_frac_i;

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
                //app_pll_frac_i = APP_PLL_F_INDEX_SYNC_22M;
                app_pll_frac = frac_values_80[APP_PLL_F_INDEX_SYNC_22M];
                //sw_pll_adder = 29184; // Count we expect on MCLK port timer at SW PLL check point. Note, we expect wrapping so this is essentiually a modulus
                break;

            case 48000 * 512:
                app_pll_ctrl = APP_PLL_CTL_SYNC_24M;
                app_pll_div = APP_PLL_DIV_SYNC_24M;
                //app_pll_frac_i = APP_PLL_F_INDEX_SYNC_24M;
                app_pll_frac = frac_values_80[APP_PLL_F_INDEX_SYNC_24M];
                //sw_pll_adder = 49152;
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

    // Write the fractional-n register
    // Set the top bit to enable the frac-n block.
    write_node_config_reg(tile, XS1_SSWITCH_SS_APP_PLL_FRAC_N_DIVIDER_NUM, (0x80000000 | app_pll_frac));

    // Wait for PLL output frequency to stabilise due to fractional divider enable
    delay_microseconds(100);

    // Write the clock divider register to enable the output
    write_node_config_reg(tile, XS1_SSWITCH_SS_APP_CLK_DIVIDER_NUM, app_pll_div);
}


void AppPllUpdate(tileref tile, unsigned short mclk_pt, struct PllSettings &pllSettings)
{
    static int mclk_pt_last;
    static int oldsetting = 0;
    static int cum_error = 0;

    unsigned sw_pll_adder = pllSettings.adder;
    unsigned app_pll_frac_i = pllSettings.fracIdx;

    const int Kp = 0;
    const int Ki = 1;

    int error, error_p, error_i;
    unsigned short expected_pt;

    if(pllSettings.firstUpdate)
    {
        mclk_pt_last = mclk_pt;  // load last mclk measurement with sensible data
        pllSettings.firstUpdate = 0;
    }
    else
    {
        int diff;

        // sw_pll_adder is the value of the port counter that we expect given the desired MCLK in the 10ms time period we are running at.
        expected_pt = mclk_pt_last + sw_pll_adder;

        // Handle wrapping
        if (porttimeafter(mclk_pt, expected_pt))
        {
            diff = -(short)(expected_pt - mclk_pt);
        }
        else
        {
            diff = (short)(mclk_pt - expected_pt);
        }

        // TODO Add a bounds checker on diff to make sure it's roughly where we expect.
        // If it isn't we should ignore it as it's either a glitch or from clock start/stop.

        error = diff; // Absolute error for last measurement cycle.
        cum_error = cum_error + error; // Integral error.

        error_p = (Kp * error);
        error_i = (Ki * cum_error);

        int newsetting = (error_p + error_i);

        // Only write new PLL settings if they're different to the old settings
        if (newsetting != oldsetting)
        {
            int frac_index = (app_pll_frac_i - newsetting) >> 2; // Tmp shift down to stop freq moving around much
            if (frac_index < 0)
            {
                frac_index = 0;
            }
            else if (frac_index > 326)
            {
                frac_index = 326;
            }

             write_node_config_reg_no_ack(tile, XS1_SSWITCH_SS_APP_PLL_FRAC_N_DIVIDER_NUM, (0x80000000 | frac_values_80[frac_index]));
        }

        oldsetting = newsetting;
        mclk_pt_last = mclk_pt;
    }
}

#endif

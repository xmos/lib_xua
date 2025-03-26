// Copyright 2017-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <platform.h>
#include "xua.h"


on tile[0]: out port p_ctrl = XS1_PORT_8D;   /* p_ctrl:
                                             * [0:3] - Unused
                                             * [4]   - EN_3v3_N    (1v0 hardware only)
                                             * [5]   - EN_3v3A
                                             * [6]   - EXT_PLL_SEL (CS2100:0, SI: 1)
                                             * [7]   - MCLK_DIR    (Out:0, In: 1)
                                             */

#define USE_FRACTIONAL_N         (0)

#if (USE_FRACTIONAL_N)
#define EXT_PLL_SEL__MCLK_DIR    (0x00)
#else
#define EXT_PLL_SEL__MCLK_DIR    (0x80)
#endif

/* Board setup for XU316 MC Audio (1v1) */
void board_setup()
{
    /* "Drive high mode" - drive high for 1, non-driving for 0 */
    set_port_drive_high(p_ctrl);

    /* Drive control port to turn on 3V3 and mclk direction appropriately.
     * Bits set to low will be high-z, pulled down */
    p_ctrl <: EXT_PLL_SEL__MCLK_DIR | 0x20;

    /* Wait for power supplies to be up and stable */
    delay_milliseconds(10);
}

/* Configures the external audio hardware at startup. Note this runs on Tile[1] */
void AudioHwInit()
{
}

/* Configures the external audio hardware for the required sample frequency */
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
}


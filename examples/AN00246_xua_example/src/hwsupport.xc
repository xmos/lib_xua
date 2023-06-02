// Copyright 2017-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <platform.h>
#include "xua.h"
#include "../../shared/apppll.h"

on tile[0]: out port p_ctrl = XS1_PORT_8D;

/* p_ctrl:
 * [0:3] - Unused
 * [4]   - EN_3v3_N
 * [5]   - EN_3v3A
 * [6]   - EXT_PLL_SEL (CS2100:0, SI: 1)
 * [7]   - MCLK_DIR    (Out:0, In: 1)
 */
#define EXT_PLL_SEL__MCLK_DIR    (0x80)

/* Note, this runs on Tile[0] */
void ctrlPort()
{
    // Drive control port to turn on 3V3 and set MCLK_DIR
    // Note, "soft-start" to reduce current spike
    // Note, 3v3_EN is inverted
    for (int i = 0; i < 30; i++)
    {
        p_ctrl <: EXT_PLL_SEL__MCLK_DIR | 0x30; /* 3v3: off, 3v3A: on */
        delay_microseconds(5);
        p_ctrl <: EXT_PLL_SEL__MCLK_DIR | 0x20; /* 3v3: on, 3v3A: on */
        delay_microseconds(5);
    }
}

/* Configures the external audio hardware at startup. Note this runs on Tile[1] */
void AudioHwInit()
{
    /* Wait for power supply to come up */
    delay_milliseconds(100);

    /* Use xCORE Secondary PLL to generate *fixed* master clock */
    AppPllEnable_SampleRate(DEFAULT_FREQ);

    delay_milliseconds(100);

	/* DAC setup: For basic I2S input we don't need any register setup. DACs will clock auto detect etc.
     * It holds DAC in reset until it gets clocks anyway.
	 * Note, this example doesn't use the ADC's
	 */
}

/* Configures the external audio hardware for the required sample frequency */
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    AppPllEnable_SampleRate(samFreq);
}


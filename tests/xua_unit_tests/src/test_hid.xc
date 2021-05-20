// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xua_unit_tests.h"

in port p_mclk_in                   = XS1_PORT_1D;

/* Clock-block declarations */
clock clk_audio_bclk                = on tile[0]: XS1_CLKBLK_1;   /* Bit clock */
clock clk_audio_mclk                = on tile[0]: XS1_CLKBLK_4;   /* Master clock */

// Supply missing but unused function
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    ; // nothing
}

// Supply missing but unused function
void AudioHwInit()
{
    ; // nothing
}

void test_null(){
    TEST_ASSERT_MESSAGE(1, "Success!");
}

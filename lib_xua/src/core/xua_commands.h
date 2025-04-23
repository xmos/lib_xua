// Copyright 2011-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_COMMANDS_H_
#define _XUA_COMMANDS_H_

#include "xua.h"

/* Clocking commands - c_clk_ctl */
#define GET_SEL                 0       /* Get value of clock selector */
#define SET_SEL                 1       /* Set value of clock selector */
#define GET_FREQ                2       /* Get current freq */
#define GET_VALID               3       /* Get current validity */
#define SET_SMUX                7       /* Set SMUX mode (ADAT) */

enum
{
    CLOCK_INTERNAL = 0,
#if XUA_SPDIF_RX_EN
    CLOCK_SPDIF,
#endif
#if XUA_ADAT_RX_EN
    CLOCK_ADAT,
#endif
    CLOCK_COUNT
};

/* c_aud_ctl commands - These are propagated from EP0 through EP buffer to Decouple. */
/* These commands will reach I2S and will cause the audioloop to break and restart*/
#define SET_SAMPLE_FREQ         4
#define SET_STREAM_FORMAT_OUT   8
#define SET_STREAM_FORMAT_IN    9

/* These commands only go as far as Decouple where they are translated to the next block */
/* Note these need to be in order in a contiguous block due to the way the code handles them in decouple */
#define SET_STREAM_INPUT_START  10
#define SET_STREAM_INPUT_STOP   11
#define SET_STREAM_OUTPUT_START 12
#define SET_STREAM_OUTPUT_STOP  13

/* These commands exist only between Decouple and Audio. They are special cases that
enable a low-power mode in Audio by allowing looping to stop and APIs to be called to
reduce power consumption */
#define SET_AUDIO_START         20
#define SET_AUDIO_STOP          21


#include "dsd_support.h"

#endif



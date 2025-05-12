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
/* Note we avoid special channel tokens - see xs1b_user.h*/
#define XUA_AUDCTL_NO_COMMAND               0 /* No command from decouple - carry on exchanging samples. This MUST be value zero */

/* These commands will reach I2S and will cause the audioloop to break and restart*/
#define XUA_AUDCTL_SET_SAMPLE_FREQ          4 

/* These commands only go as far as Decouple where they are translated to the next block */
/* Note these need to be in order in a contiguous block due to the way the code handles them in decouple */
/* When starting a stream, format information is also sent */
#define XUA_AUDCTL_SET_STREAM_INPUT_START   20
#define XUA_AUDCTL_SET_STREAM_INPUT_STOP    21
#define XUA_AUDCTL_SET_STREAM_OUTPUT_START  22
#define XUA_AUDCTL_SET_STREAM_OUTPUT_STOP   23

/* These commands exist only between Decouple and Audio. The logic to translate from the previous block is in decouple */
#define XUA_AUD_SET_AUDIO_START             30
#define XUA_AUD_SET_AUDIO_STOP              31


#include "dsd_support.h"

#endif



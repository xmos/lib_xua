// Copyright 2011-2023 XMOS LIMITED.
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

/* c_audioControl */
#define SET_SAMPLE_FREQ         4
#define SET_STREAM_FORMAT_OUT   8
#define SET_STREAM_FORMAT_IN    9

#include "dsd_support.h"

#endif



// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __XUA_INTERNAL_CMDS_H__
#define __XUA_INTERNAL_CMDS_H__

#include "xua.h"

/* Clocking commands - c_clk_ctl */
#define GET_SEL                 0       /* Get value of clock selector */
#define SET_SEL                 1       /* Set value of clock selector */
#define GET_FREQ                2       /* Get current freq */
#define GET_VALID               3       /* Get current validity */

#define CLOCK_INTERNAL          1
#define CLOCK_SPDIF             2
#if SPDIF_RX
#define CLOCK_ADAT              3
#else
#define CLOCK_ADAT              2
#endif

#define CLOCK_INTERNAL_INDEX    (CLOCK_INTERNAL - 1)
#define CLOCK_ADAT_INDEX        (CLOCK_ADAT - 1)
#define CLOCK_SPDIF_INDEX       (CLOCK_SPDIF - 1)

#define SET_SMUX                7

/* c_audioControl */
#define SET_SAMPLE_FREQ         4
#define SET_STREAM_FORMAT_OUT   8
#define SET_STREAM_FORMAT_IN    9

#include "dsd_support.h"

#endif



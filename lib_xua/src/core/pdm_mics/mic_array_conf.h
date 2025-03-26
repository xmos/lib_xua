// Copyright 2015-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef MIC_ARRAY_CONF_H_
#define MIC_ARRAY_CONF_H_

#include "xua_conf_full.h"

#if XUA_NUM_PDM_MICS > 0

/* PDM mics configuration */
#ifndef MIC_ARRAY_CONFIG_MCLK_FREQ
#define MIC_ARRAY_CONFIG_MCLK_FREQ          MCLK_48             // Used with MIC_ARRAY_CONFIG_PDM_FREQ to calculate clock divider 
#endif
#ifndef MIC_ARRAY_CONFIG_PDM_FREQ
#define MIC_ARRAY_CONFIG_PDM_FREQ           3072000             // Used with MIC_ARRAY_CONFIG_MCLK_FREQ to calculate clock divider 
#endif
#ifndef MIC_ARRAY_CONFIG_USE_DC_ELIMINATION
#define MIC_ARRAY_CONFIG_USE_DC_ELIMINATION 1                   // DC filter
#endif
#ifndef MIC_ARRAY_CONFIG_USE_DDR
#define MIC_ARRAY_CONFIG_USE_DDR            1                   // Two mics on one data line by default
#endif
#ifndef MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME
#define MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME  1                   // XUA is sample based
#endif
#define MIC_ARRAY_CONFIG_MIC_COUNT          XUA_NUM_PDM_MICS
#define MIC_ARRAY_CONFIG_MIC_IN_COUNT       XUA_NUM_PDM_MICS

/* PDM mics resources */
#ifndef MIC_ARRAY_CONFIG_PORT_MCLK
#error Please define MIC_ARRAY_CONFIG_PORT_MCLK for PDM mics
#endif
#ifndef MIC_ARRAY_CONFIG_PORT_PDM_CLK
#error Please define MIC_ARRAY_CONFIG_PORT_PDM_CLK for PDM mics
#endif
#ifndef MIC_ARRAY_CONFIG_PORT_PDM_DATA
#error Please define MIC_ARRAY_CONFIG_PORT_PDM_DATA for PDM mics
#endif
#ifndef MIC_ARRAY_CONFIG_CLOCK_BLOCK_A
#error Please define MIC_ARRAY_CONFIG_CLOCK_BLOCK_A for PDM mics
#endif
#if MIC_ARRAY_CONFIG_USE_DDR && !defined(MIC_ARRAY_CONFIG_CLOCK_BLOCK_B)
#error Please define MIC_ARRAY_CONFIG_CLOCK_BLOCK_B for PDM mics
#endif

/* Test for errors in config */
#ifndef XUA_PDM_MIC_FREQ
#error Please define XUA_PDM_MIC_FREQ when using PDM mics (single output sample rate only supported currently)
#endif

#if (MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME != 1)
#error ONLY MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME = 1 supported currently
#endif

#endif /* XUA_NUM_PDM_MICS > 0 */

#endif /* MIC_ARRAY_CONF_H_ */

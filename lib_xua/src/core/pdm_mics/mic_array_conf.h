// Copyright 2015-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef MIC_ARRAY_CONF_H_
#define MIC_ARRAY_CONF_H_

#include "xua_conf_full.h"

#if XUA_NUM_PDM_MICS > 0
/* PDM mics configuration */

#define MIC_ARRAY_CONFIG_MIC_COUNT          XUA_NUM_PDM_MICS
#define MIC_ARRAY_CONFIG_MIC_IN_COUNT       XUA_NUM_PDM_MICS_IN

#define MIC_ARRAY_CONFIG_USE_PDM_ISR        XUA_PDM_MIC_USE_PDM_ISR

#endif /* XUA_NUM_PDM_MICS > 0 */

#endif /* MIC_ARRAY_CONF_H_ */

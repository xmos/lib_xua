// Copyright 2017-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __XUA_H__
#define __XUA_H__

#include <xs1.h>
#include "xua_conf_full.h"

#if __XC__ || __STDC__
    #include "xua_audiohub.h"

    #include "xua_endpoint0.h"

    #include "xua_buffer.h"
#endif

#if __XC__
    #if XUA_NUM_PDM_MICS > 0
        #include "xua_pdm_mic.h"
    #endif
#endif

#endif

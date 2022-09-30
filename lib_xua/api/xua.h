// Copyright 2017-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_H_
#define _XUA_H_

#include <xs1.h>

#include "xua_conf_full.h"

#if __XC__ || __STDC__
    #include "xua_audiohub.h"
    #include "xua_endpoint0.h"
    #include "xua_buffer.h"
    #include "xua_mixer.h"
#endif

#if __XC__
    #include "xua_clocking.h"
    #include "xua_midi.h"
    #if XUA_NUM_PDM_MICS > 0
        #include "xua_pdm_mic.h"
    #endif
#endif

#endif

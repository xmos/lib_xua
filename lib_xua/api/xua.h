// Copyright (c) 2017-2018, XMOS Ltd, All rights reserved

#ifndef __XUA_H__
#define __XUA_H__

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

// Copyright 2017-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_H_
#define _XUA_H_

#include <xs1.h>

#ifndef __ASSEMBLER__
#include <xccompat.h>

/* Missing items from xccompat.h */
#if defined( __XC__)
    #define NULLABLE_CLIENT_INTERFACE(tag, name) client interface tag ?name
    #define NULLABLE_SERVER_INTERFACE(tag, name) server interface tag ?name
    typedef in port in_port_t;
    typedef out port out_port_t;
#elif defined(__STDC__) || defined(__DOXYGEN__)
    #define NULLABLE_CLIENT_INTERFACE(type, name) unsigned *name
    #define NULLABLE_SERVER_INTERFACE(tag, name) unsigned *name
    typedef unsigned clock;
    typedef unsigned in_port_t;
    typedef unsigned out_port_t;
#endif
#endif

#include "xua_conf_full.h"

#ifndef __ASSEMBLER__
    #include "xua_audiohub.h"
    #include "xua_endpoint0.h"
    #include "xua_buffer.h"
    #include "xua_mixer.h"
#endif

#ifdef __XC__
    #include "xua_clocking.h"
    #include "xua_midi.h"
    #if XUA_NUM_PDM_MICS > 0
        #include "xua_pdm_mic.h"
    #endif
#endif

#endif

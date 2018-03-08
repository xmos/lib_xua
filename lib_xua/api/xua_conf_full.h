// Copyright (c) 2017-2018, XMOS Ltd, All rights reserved
#ifndef __XUA_CONF_FULL_H__
#define __XUA_CONF_FULL_H__

#ifdef __xua_conf_h_exists__
    #include "xua_conf.h"
#endif

#include "xua_conf_default.h"

#if CODEC_MASTER
    #define _XUA_CLK_DIR in
#else
    #define _XUA_CLK_DIR out
#endif

#endif

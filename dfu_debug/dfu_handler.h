// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __dfu_handler_h__
#define __dfu_handler_h__

#include <xccompat.h>

#ifdef __XC__
[[distributable]] [[combinable]]
#endif
void DFUHandler(SERVER_INTERFACE(i_dfu, i),
                NULLABLE_RESOURCE(chanend, c_user_cmd));

#endif

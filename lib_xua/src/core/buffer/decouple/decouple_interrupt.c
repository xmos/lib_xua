// Copyright (c) 2015-2018, XMOS Ltd, All rights reserved
#include "xua.h"
#if XUA_USB_EN
#include "interrupt.h"

register_interrupt_handler(handle_audio_request, 1, 200)
#endif

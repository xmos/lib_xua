// Copyright 2024-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

#ifdef __XC__

#include "i2c.h"
#include <platform.h>
#include <xk_audio_316_mc_ab/board.h>

extern unsafe client interface i2c_master_if i_i2c_client;
extern void board_setup();
extern void power_down();

#ifdef LOW_POWER_ENABLE
/* Call the clock power down code as per https://github.com/xmosnotes/an01009
*/
#define POWER_DOWN() power_down()
#else
/* Do nothing */
#define POWER_DOWN()
#endif

#define USER_MAIN_DECLARATIONS \
    interface i2c_master_if i2c[1];

#define USER_MAIN_CORES on tile[0]: {\
                                        par{\
                                            {\
                                                board_setup();\
                                                xk_audio_316_mc_ab_i2c_master(i2c);\
                                            }\
                                            POWER_DOWN();\
                                        }\
                                    }\
                        on tile[1]: {\
                                        unsafe\
                                        {\
                                            i_i2c_client = i2c[0]; \
                                        }\
                                    }
#endif

#endif

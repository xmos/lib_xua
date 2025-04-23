// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

#ifdef __XC__
#include "xk_evk_xu316/board.h"

#define USER_MAIN_DECLARATIONS				chan c_i2c;                                         /* Channel for connecting I2C to hwsupport callbacks */
#define USER_MAIN_CORES 					on tile[0]: xk_evk_xu316_AudioHwRemote(c_i2c); 	    /* Startup remote I2C master server task */ \
											on tile[1]: xk_evk_xu316_AudioHwChanInit(c_i2c);    /* Initialise the channel-end task */

#endif // __XC__
#endif // _USER_MAIN_H_

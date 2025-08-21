// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _xua_pwm_conf_default_h_
#define _xua_pwm_conf_default_h_

#ifdef __xua_pwm_conf_h_exists__
    #include "xua_pwm_conf.h"
#endif

#ifndef XUA_PWM_SD_COEFFS
#define XUA_PWM_SD_COEFFS     sd_coeffs_o6_f1_5_n8
#endif

#ifndef XUA_PWM_SCALE
#define XUA_PWM_SCALE         2.8544
#endif

#ifndef XUA_PWM_LIMIT
#define XUA_PWM_LIMIT         2.8684735298
#endif

#ifndef XUA_PWM_FLAT_COMP_X2
#define XUA_PWM_FLAT_COMP_X2  (-1.0/29000)
#endif

#ifndef XUA_PWM_FLAT_COMP_X3
#define XUA_PWM_FLAT_COMP_X3  (1.0/120000)
#endif

#ifndef XUA_PWM_PWM_COMP_X2
#define XUA_PWM_PWM_COMP_X2   (-3.0/190)
#endif

#ifndef XUA_PWM_PWM_COMP_X3
#define XUA_PWM_PWM_COMP_X3   (0.63/80)
#endif

#ifndef XUA_PWM_NEGATE
#define XUA_PWM_NEGATE        1
#endif

#endif

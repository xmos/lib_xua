#ifndef __pwm_thread__h_
#define __pwm_thread__h_

#include "xua_conf_default.h" // Settings

#if (XUA_PWM_CHANNELS > 0)
  #ifdef __XC__
void pwm_thread(chanend c_data, in port p_mclk);
  #else
    #include <xcore/chanend.h>
void pwm_thread(chanend_t c_data, port_t p_mclk);
  #endif
#endif

#endif




/**
 * @file    pwm_thread.c
 * @brief   Fork the threads to run a PWM engine
 * @author  Henk Muller, XMOS Semiconductor Ltd
 */
#include "xua_conf_default.h" // Settings
#if (XUA_PWM_CHANNELS > 0)
#include <platform.h>
#include <xcore/port.h>
#include <xcore/chanend.h>
#include <xcore/clock.h>
#include "software_dac_hp.h"
#include "sigma_delta_modulators.h"
#include "uac_hwresources.h"

void setup_master_clock(port_t clk_in) __attribute__ ((weak));
void setup_master_clock(port_t clk_in) {
}

void pwm_init(port_t clk_in) {
    xclock_t clk = CLKBLK_PWM;
    setup_master_clock(clk_in);
    port_set_invert(clk_in);
    clock_enable(clk);
    clock_set_source_port(clk, clk_in);
}

void pwm_thread(chanend_t c_data)
{
    software_dac_hp_t sd;
    xclock_t clk = CLKBLK_PWM;
    port_t ports[2] = {PORT_PWM_OUT_LEFT, PORT_PWM_OUT_RIGHT};
    port_t clk_out = PORT_PWM_CLK_OUT;

    software_dac_hp_init(&sd, ports, clk, clk_out, 8, sd_coeffs_o6_f1_5_n8);

    software_dac_hp(&sd, c_data);
}
#endif




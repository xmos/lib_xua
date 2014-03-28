
#if (XUD_SERIES_SUPPORT==1)
#include "archU_powerSaving.h"
#include <xs1.h>
#include <xs1_l_registers.h>
#include <xs1_su_registers.h>
#include <platform.h>

#if (XCC_MAJOR_VERSION >= 1300)
#include <hwtimer.h>
#else
#define hwtimer_t timer
#endif

#ifndef VOLTAGE_REDUCTION_mV
#define VOLTAGE_REDUCTION_mV 20
#endif

unsigned get_tile_id(tileref t); // Required for use with 12.0 tools. get_tile_id() available in xs1.h with 13.0 tools.

#define ARCH_U_VOLTAGE_FIRST_STEP 39 // First step down from 1V
#define ARCH_U_SSWITCH_PRESCALER 8 // Sswitch down to 1/8 clk freq

void archU_powerSaving()
{
    if (get_local_tile_id() == get_tile_id(tile[0]))
    {
        unsigned writeval[1];
        unsigned char writevalc[1];

        // Reduce the VDDCORE voltage level
        for (unsigned count=0; count < (VOLTAGE_REDUCTION_mV/10); count++)
        {
            hwtimer_t t;
            int time;

            writeval[0] = (ARCH_U_VOLTAGE_FIRST_STEP - count);
            write_periph_32(usb_tile, XS1_SU_PER_PWR_CHANEND_NUM, XS1_SU_PER_PWR_VOUT1_LVL_NUM, 1, writeval);

            t :> time;
            time += (1 * PLATFORM_REFERENCE_MHZ); // Wait 1us per step
            t when timerafter(time) :> void;
        }

        // Set switch prescaler down
        write_node_config_reg(tile[0], XS1_SSWITCH_CLK_DIVIDER_NUM, (ARCH_U_SSWITCH_PRESCALER - 1)); // PLL clk will be divided by value + 1

        // Both DC-DCs in PWM mode, I/O and PLL supply on, Analogue & core on
        writeval[0] = XS1_SU_PWR_VOUT1_EN_SET(0, 1);
        writeval[0] = XS1_SU_PWR_VOUT2_EN_SET(writeval[0], 1);
        writeval[0] = XS1_SU_PWR_VOUT5_EN_SET(writeval[0], 1);
        writeval[0] = XS1_SU_PWR_VOUT6_EN_SET(writeval[0], 1);
        write_periph_32(usb_tile, XS1_SU_PER_PWR_CHANEND_NUM, XS1_SU_PER_PWR_STATE_AWAKE_NUM, 1, writeval);

        // USB suspend off, voltage adjustable, sleep clock 32KHz
        writeval[0] = XS1_SU_PWR_USB_PD_EN_SET(0, 1);
        writeval[0] = XS1_SU_PWR_RST_VOUT_LVL_SET(writeval[0], 1);
        writeval[0] = XS1_SU_PWR_LD_AWAKE_SET(writeval[0], 1);
        write_periph_32(usb_tile, XS1_SU_PER_PWR_CHANEND_NUM, XS1_SU_PER_PWR_MISC_CTRL_NUM, 1, writeval);

        // Turn off on-chip silicon oscillator (20MHz or 32KHz)
        writevalc[0] = XS1_SU_ON_SI_OSC_EN_SET(0, 1);
        writevalc[0] = XS1_SU_ON_SI_OSC_SLOW_SET(writevalc[0], 1);
        write_periph_8(usb_tile, XS1_SU_PER_OSC_CHANEND_NUM, XS1_SU_PER_OSC_ON_SI_CTRL_NUM, 1, writevalc);
    }
}
#endif


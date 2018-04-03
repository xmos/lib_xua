// Copyright (c) 2016-2018, XMOS Ltd, All rights reserved

#include <platform.h>
#include <timer.h>
#include <stdint.h>

#include "xua.h"
#include "i2c.h"     /* From lib_i2c */

#include "cs5368.h"
#include "cs4384.h"

port p_i2c = on tile[0]:XS1_PORT_4A;

/* General output port bit definitions */
#define P_GPIO_DSD_MODE         (1 << 0) /* DSD mode select 0 = 8i/8o I2S, 1 = 8o DSD*/
#define P_GPIO_DAC_RST_N        (1 << 1)
#define P_GPIO_USB_SEL0         (1 << 2)
#define P_GPIO_USB_SEL1         (1 << 3)
#define P_GPIO_VBUS_EN          (1 << 4)
#define P_GPIO_PLL_SEL          (1 << 5) /* 1 = CS2100, 0 = Phaselink clock source */
#define P_GPIO_ADC_RST_N        (1 << 6)
#define P_GPIO_MCLK_FSEL        (1 << 7) /* Select frequency on Phaselink clock. 0 = 24.576MHz for 48k, 1 = 22.5792MHz for 44.1k.*/

#define DAC_REGWRITE(reg, val) result = i2c.write_reg(CS4384_I2C_ADDR, reg, val);
#define DAC_REGREAD(reg)  data = i2c.read_reg(CS4384_I2C_ADDR, reg, result);
#define ADC_REGWRITE(reg, val) result = i2c.write_reg(CS5368_I2C_ADDR, reg, val);

out port p_gpio = on tile[0]:XS1_PORT_8C;

void AudioHwConfig2(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC, client interface i2c_master_if i2c)
{
	unsigned char gpioVal = 0;
    i2c_regop_res_t result;

    /* Set master clock select appropriately and put ADC and DAC into reset */
    if (mClk == MCLK_441)
    {
		gpioVal =  P_GPIO_USB_SEL0 | P_GPIO_USB_SEL1;
    }
    else
    {
		gpioVal = P_GPIO_USB_SEL0 | P_GPIO_USB_SEL1 | P_GPIO_MCLK_FSEL;
    }

	p_gpio <: gpioVal;

    /* Allow MCLK to settle */
    delay_microseconds(20000);

	/* Take ADC out of reset */
	gpioVal |= P_GPIO_ADC_RST_N;        
	p_gpio <: gpioVal;

	/* Configure ADC for I2S slave mode via I2C */
	unsigned dif = 0, mode = 0;
	dif = 0x01;   	/* I2S */
	mode = 0x03;    /* Slave mode all speeds */

	/* Reg 0x01: (GCTL) Global Mode Control Register 
	* Bit[7]: CP-EN: Manages control-port mode
	* Bit[6]: CLKMODE: Setting puts part in 384x mode
	* Bit[5:4]: MDIV[1:0]: Set to 01 for /2
	* Bit[3:2]: DIF[1:0]: Data Format: 0x01 for I2S, 0x02 for TDM
	* Bit[1:0]: MODE[1:0]: Mode: 0x11 for slave mode
	*/
	ADC_REGWRITE(CS5368_GCTL_MDE, 0b10010000 | (dif << 2) | mode);


	/* Reg 0x06: (PDN) Power Down Register */
	/* Bit[7:6]: Reserved
	 * Bit[5]: PDN-BG: When set, this bit powers-own the bandgap reference
	 * Bit[4]: PDM-OSC: Controls power to internal oscillator core
	 * Bit[3:0]: PDN: When any bit is set all clocks going to that channel pair are turned off
	 */
	ADC_REGWRITE(CS5368_PWR_DN, 0b00000000);

	/* Configure DAC with PCM values. Note 2 writes to mode control to enable/disable freeze/power down */
	/* Take DAC out of reset */
	gpioVal |= P_GPIO_DAC_RST_N;
	p_gpio <: gpioVal;

	delay_microseconds(500);

	/* Mode Control 1 (Address: 0x02) */
	/* bit[7] : Control Port Enable (CPEN)     : Set to 1 for enable
	 * bit[6] : Freeze controls (FREEZE)       : Set to 1 for freeze
	 * bit[5] : PCM/DSD Selection (DSD/PCM)    : Set to 0 for PCM
	 * bit[4:1] : DAC Pair Disable (DACx_DIS)  : All Dac Pairs enabled
	 * bit[0] : Power Down (PDN)               : Powered down
	 */
	DAC_REGWRITE(CS4384_MODE_CTRL, 0b11000001);
	
    /* PCM Control (Address: 0x03) */
	/* bit[7:4] : Digital Interface Format (DIF) : 0b0001 for I2S up to 24bit
	 * bit[3:2] : Reserved
	 * bit[1:0] : Functional Mode (FM) : 0x00 - single-speed mode (4-50kHz)
	 *                                 : 0x01 - double-speed mode (50-100kHz)
	 *                                 : 0x10 - quad-speed mode (100-200kHz)
	 *                                 : 0x11 - auto-speed detect (32 to 200kHz)
	 *                                 (note, some Mclk/SR ratios not supported in auto)
	 *
	 */
	unsigned char regVal = 0;
	if(samFreq < 50000)
		regVal = 0b00010100;
	else if(samFreq < 100000)
		regVal = 0b00010101;
	else //if(samFreq < 200000)
		regVal = 0b00010110;

	DAC_REGWRITE(CS4384_PCM_CTRL, regVal);

	/* Mode Control 1 (Address: 0x02) */
	/* bit[7] : Control Port Enable (CPEN)     : Set to 1 for enable
	 * bit[6] : Freeze controls (FREEZE)       : Set to 0 for freeze
	 * bit[5] : PCM/DSD Selection (DSD/PCM)    : Set to 0 for PCM
	 * bit[4:1] : DAC Pair Disable (DACx_DIS)  : All Dac Pairs enabled
	 * bit[0] : Power Down (PDN)               : Not powered down
	 */
	DAC_REGWRITE(CS4384_MODE_CTRL, 0b10000000);

    /* Kill the i2c task */
    i2c.shutdown();
    return;
}

void AudioHwInit()
{
    /* Set USB Mux to micro-b */
    /* ADC and DAC in reset */
    p_gpio <: P_GPIO_USB_SEL0 | P_GPIO_USB_SEL1;
}

void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode,
    unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    i2c_master_if i2c[1];
    par
    {
        i2c_master_single_port(i2c, 1, p_i2c, 10, 0, 1, 0);
        AudioHwConfig2(samFreq, mClk, dsdMode, sampRes_DAC, sampRes_ADC, i2c[0]);
    }
}


// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <timer.h>

#include "xua.h"

/* General output port bit definitions */
#define P_GPIO_DSD_MODE         (1 << 0) /* DSD mode select 0 = 8i/8o I2S, 1 = 8o DSD*/
#define P_GPIO_DAC_RST_N        (1 << 1)
#define P_GPIO_USB_SEL0         (1 << 2)
#define P_GPIO_USB_SEL1         (1 << 3)
#define P_GPIO_VBUS_EN          (1 << 4)
#define P_GPIO_PLL_SEL          (1 << 5) /* 1 = CS2100, 0 = Phaselink clock source */
#define P_GPIO_ADC_RST_N        (1 << 6)
#define P_GPIO_MCLK_FSEL        (1 << 7) /* Select frequency on Phaselink clock. 0 = 24.576MHz for 48k, 1 = 22.5792MHz for 44.1k.*/


out port p_gpio = on tile[0]:XS1_PORT_8C;

void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode,
    unsigned sampRes_DAC, unsigned sampRes_ADC)
{
	unsigned char gpioVal = 0;

    /* Set master clock select appropriately and put ADC and DAC into reset */
    if (mClk == MCLK_441)
    {
		gpioVal =  P_GPIO_USB_SEL0 | P_GPIO_USB_SEL1;
    }
    else
    {
		gpioVal = P_GPIO_USB_SEL0 | P_GPIO_USB_SEL1 | P_GPIO_MCLK_FSEL;
    }

    /* Note, DAC and ADC held in reset */
	p_gpio <: gpioVal;

    /* Allow MCLK to settle */
    delay_microseconds(20000);

    return;
}

void AudioHwInit()
{
    /* Set USB Mux to micro-b */
    /* ADC and DAC in reset */
    p_gpio <: P_GPIO_USB_SEL0 | P_GPIO_USB_SEL1;
}


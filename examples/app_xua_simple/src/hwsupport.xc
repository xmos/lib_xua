// Copyright (c) 2016, XMOS Ltd, All rights reserved

#include <platform.h>
#include "audiohw.h"
#include "customdefines.h"
#include "gpio_access.h"


void AudioHwConfig(unsigned samFreq, unsigned mClk, chanend ?c_codec, unsigned dsdMode,
    unsigned sampRes_DAC, unsigned sampRes_ADC)
{
  // nothing
}

void AudioHwInit(chanend ?c_codec)
{
    set_gpio(P_GPIO_USB_SEL0, 1);
    set_gpio(P_GPIO_USB_SEL1, 1);
}


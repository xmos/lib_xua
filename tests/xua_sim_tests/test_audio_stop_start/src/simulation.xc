// Copyright 2016-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifdef SIMULATION

#include <platform.h>
#include <print.h>

extern port p_mclk25mhz;
extern clock clk_mclk25mhz;


extern clock clk_audio_mclk_gen;
extern out port p_mclk_gen;
void master_mode_clk_setup(void)
{
  configure_clock_rate(clk_audio_mclk_gen, 25, 1); // Slighly faster than typical MCLK of 24.576MHz
  configure_port_clock_output(p_mclk_gen, clk_audio_mclk_gen);
  start_clock(clk_audio_mclk_gen);

  //printstrln("Starting mclk");
  delay_seconds(-1); //prevent destructor ruining clock gen
}

#endif

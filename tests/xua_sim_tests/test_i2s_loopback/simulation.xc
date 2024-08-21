// Copyright 2016-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifdef SIMULATION

#include <platform.h>
#include <print.h>

extern port p_mclk_in;

extern port p_mclk25mhz;
extern clock clk_mclk25mhz;

void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
  // nothing
}

void AudioHwInit()
{
  // nothing
}


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


#if CODEC_MASTER
extern out port  p_bclk_gen;
extern clock clk_audio_bclk_gen;
extern out port  p_lrclk_gen;
extern clock clk_audio_lrclk_gen;

void slave_mode_clk_setup(const unsigned samFreq, const unsigned chans_per_frame){
  const unsigned data_bits = XUA_I2S_N_BITS;
  const unsigned mclk_freq = 24576000;

  const unsigned mclk_bclk_ratio = mclk_freq / (chans_per_frame * samFreq * data_bits);
  const unsigned bclk_lrclk_ratio = (chans_per_frame * data_bits); // 48.828Hz  LRCLK

  //bclk
  configure_clock_src_divide(clk_audio_bclk_gen, p_mclk_gen, mclk_bclk_ratio/2);
  configure_port_clock_output(p_bclk_gen, clk_audio_bclk_gen);
  start_clock(clk_audio_bclk_gen);

  //lrclk
  configure_clock_src_divide(clk_audio_lrclk_gen, p_bclk_gen, bclk_lrclk_ratio/2);
  configure_port_clock_output(p_lrclk_gen, clk_audio_lrclk_gen);
  start_clock(clk_audio_lrclk_gen);

  //mclk
  master_mode_clk_setup();
}
#endif
#endif

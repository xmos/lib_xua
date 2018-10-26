#include "i2s.h"
#include "i2c.h"
#include "gpio.h"
#include "xua.h"
#define DEBUG_UNIT XUA_AUDIO_HUB
#define DEBUG_PRINT_ENABLE_XUA_AUDIO_HUB 1
#include "debug_print.h"


[[distributable]]
void AudioHub(server i2s_frame_callback_if i2s,
                  chanend c_aud,
                  client i2c_master_if ?i_i2c,
                  client output_gpio_if dac_reset)
{
  int32_t samples[8] = {0}; // Array used for looping back samples
  

  // Set reset DAC
  dac_reset.output(0);
  delay_milliseconds(1);
  dac_reset.output(1);


  while (1) {
    select {
    case i2s.init(i2s_config_t &?i2s_config, tdm_config_t &?tdm_config):
      i2s_config.mode = I2S_MODE_I2S;
      i2s_config.mclk_bclk_ratio = (MCLK_48/DEFAULT_FREQ)/64;

      debug_printf("I2S init\n");
      break;

    case i2s.receive(size_t n_chans, int32_t in_samps[n_chans]):
    for (int i = 0; i < n_chans; i++) samples[i] = in_samps[i]; // copy samples
      break;

    case i2s.send(size_t n_chans, int32_t out_samps[n_chans]):
      for (int i = 0; i < n_chans; i++) out_samps[i] = samples[i]; // copy samples
      break;

    case i2s.restart_check() -> i2s_restart_t restart:
      restart = I2S_NO_RESTART; // Keep on looping
      break;
    }
  }
}

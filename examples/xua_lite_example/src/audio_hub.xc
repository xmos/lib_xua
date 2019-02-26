#include <string.h>
#include "i2s.h"
#include "i2c.h"
#include "xua.h"
#define DEBUG_UNIT XUA_AUDIO_HUB
#define DEBUG_PRINT_ENABLE_XUA_AUDIO_HUB 1
#include "debug_print.h"
#include "mic_array.h"
#include "audio_config.h"
#include "pdm_mic.h"

//Globally declared for 64b alignment
int mic_decimator_fir_data_array[8][THIRD_STAGE_COEFS_PER_STAGE * PDM_MAX_DECIMATION] = {{0}};
mic_array_frame_time_domain mic_audio_frame[2];

[[distributable]]
void AudioHub(server i2s_frame_callback_if i2s,
                  streaming chanend c_audio,
                  streaming chanend (&?c_ds_output)[1])
{
  int32_t samples_out[NUM_USB_CHAN_OUT] = {0};
  int32_t samples_in[NUM_USB_CHAN_IN] = {0};

  int32_t clock_nudge = 0;

  //PDM mic and decimator
  unsigned buffer;
  int raw_mics[XUA_NUM_PDM_MICS] = {0};
  const unsigned decimatorCount = 1; // Supports up to 4 mics
  mic_array_decimator_conf_common_t dcc;
  mic_array_decimator_config_t dc[1];
  mic_array_frame_time_domain * unsafe current;

  mic_array_decimator_set_samprate(DEFAULT_FREQ, mic_decimator_fir_data_array[0], &dcc, dc);
  mic_array_decimator_configure(c_ds_output, decimatorCount, dc);
  mic_array_init_time_domain_frame(c_ds_output, decimatorCount, buffer, mic_audio_frame, dc);

  // Used for debug
  //int saw = 0;

  while (1) {
    select {
    case i2s.init(i2s_config_t &?i2s_config, tdm_config_t &?tdm_config):
      i2s_config.mode = I2S_MODE_I2S;
      i2s_config.mclk_bclk_ratio = (MCLK_48/DEFAULT_FREQ)/64;
      debug_printf("I2S init\n");
      delay_milliseconds(500); //Work around to ensure I2S does not start until enumeration complete so timing does not break for exchange
                               //This should be ideally done by set config by the host (via xua_buffer) to know we are enumerated
    break;

    case i2s.receive(size_t n_chans, int32_t in_samps[n_chans]):
      for (int i = 0; i < n_chans; i++) samples_in[i] = in_samps[i];
    break;

    case i2s.send(size_t n_chans, int32_t out_samps[n_chans]):
      for (int i = 0; i < n_chans; i++) out_samps[i] = samples_out[i];
    break;

    //Exchange samples with mics & host
    case i2s.restart_check() -> i2s_restart_t restart:
      restart = I2S_NO_RESTART; // Keep on looping
      timer tmr; int t0, t1; tmr :> t0;

      //Transfer samples. Takes about 25 ticks
      for (int i = 0; i < NUM_USB_CHAN_OUT; i++) c_audio :> samples_out[i];
      if (XUA_ADAPTIVE) c_audio :> clock_nudge;
      for (int i = 0; i < NUM_USB_CHAN_IN; i++) c_audio <: raw_mics[i];

      //Grab mics. Takes about 200 ticks currently
      current = mic_array_get_next_time_domain_frame(c_ds_output, decimatorCount, buffer, mic_audio_frame, dc);
      //50 ticks
      unsafe {
          for (int i = 0; i < XUA_NUM_PDM_MICS; i++) raw_mics[i] = current->data[i][0];
      }

      //memset(raw_mics, saw, sizeof(int) * XUA_NUM_PDM_MICS);
      //saw += 500;

      //Taking about 160 ticks when adjusting, 100 when not
      tmr :> t0;
      pll_nudge(clock_nudge);
      tmr :> t1;
      if (t1-t0 > 500) debug_printf("*%d\n", t1 - t0);
      //delay_microseconds(10); //Test backpressure tolerance
    break;
    }
  }
}

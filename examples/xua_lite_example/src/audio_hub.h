#include "i2s.h"

[[distributable]]
void AudioHub(server i2s_frame_callback_if i2s, streaming chanend c_audio, streaming chanend (&?c_ds_output)[1]);
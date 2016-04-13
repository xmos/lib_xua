
#include "mic_array.h"

void pcm_pdm_mic(chanend c_pcm_out);

#ifdef MIC_PROCESSING_USE_INTERFACE
/* Interface based user processing */
typedef interface mic_process_if
{
    //[[guarded]]
    //void transfer_buffers(int * unsafe in_mic_buf, int * unsafe in_spk_buf, int * unsafe out_mic_buf, int * unsafe out_spk_buf);
    void transfer_buffers(mic_array_frame_time_domain * unsafe audio, int output[]);

    void init();
} mic_process_if;

[[distributable]]
unsafe void user_pdm_process(server mic_process_if i_mic_data);

#else

/* Simple user hooks/call-backs */
unsafe void user_pdm_process(mic_array_frame_time_domain * unsafe audio, int output[]);

void user_pdm_init();

#endif



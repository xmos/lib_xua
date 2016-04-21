
#include "mic_array.h"

#ifdef MIC_PROCESSING_USE_INTERFACE
/* Interface based user processing */
typedef interface mic_process_if
{
    //[[guarded]]
    //void transfer_buffers(int * unsafe in_mic_buf, int * unsafe in_spk_buf, int * unsafe out_mic_buf, int * unsafe out_spk_buf);
    void transfer_buffers(mic_array_frame_time_domain * unsafe audio, int output[]);

    void init();
} mic_process_if;


[[combinable]]
void pdm_process(streaming chanend c_ds_output[2], chanend c_audio
#ifdef MIC_PROCESSING_USE_INTERFACE
   , client mic_process_if i_mic_process
#endif
);

[[combinable]]
void user_pdm_process(server mic_process_if i_mic_data);


void pcm_pdm_mic(streaming chanend c_ds_output[2]);

#else

/* Simple user hooks/call-backs */
unsafe void user_pdm_process(mic_array_frame_time_domain * unsafe audio, int output[]);

void user_pdm_init();

void pcm_pdm_mic(streaming chanend c_ds_output[2]);

#endif


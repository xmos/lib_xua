
#include "mic_array.h"

#ifdef MIC_PROCESSING_USE_INTERFACE
/* Interface based user processing */
typedef interface mic_process_if
{
    void transfer_buffers(mic_array_frame_time_domain * unsafe audio);
    void init();
} mic_process_if;


[[combinable]]
void pdm_buffer(streaming chanend c_ds_output[2], chanend c_audio
#ifdef MIC_PROCESSING_USE_INTERFACE
   , client mic_process_if i_mic_process
#endif
);

[[combinable]]
void user_pdm_process(server mic_process_if i_mic_data);

/* PDM interface and decimation cores */
void pdm_mic(streaming chanend c_ds_output[2]);

#else

/* Simple user hooks/call-backs */
void user_pdm_process(mic_array_frame_time_domain * unsafe audio);

void user_pdm_init();

/* PDM interface and decimation cores */
[[combinable]]
void pdm_buffer(streaming chanend c_ds_output[2], chanend c_audio);

/* PDM interface and decimation cores */
void pdm_mic(streaming chanend c_ds_output[2]);

#endif


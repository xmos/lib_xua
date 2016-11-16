
#include "devicedefines.h"

#if (NUM_PDM_MICS > 0) && !defined(MIC_PROCESSING_USE_INTERFACE)

#include "mic_array_frame.h" 

/* Deafult implementations of user_pdm_init() and user_pdm_process().  Both can be over-ridden */
void user_pdm_init() __attribute__ ((weak));
void user_pdm_init()
{
    return;
}


void user_pdm_process() __attribute__ ((weak));
void user_pdm_process(mic_array_frame_time_domain * audio, int output[])
{

    for(unsigned i=0; i<NUM_PDM_MICS; i++)
    {
        /* Simply copy input buffer to output buffer unmodified */
        output[i] = audio->data[i][0];
    }

    return;
}

#endif


#include "xccompat.h"
#include "devicedefines.h"

/* Default implentation for UserBufferManagement() */
void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[] 
#ifdef RUN_DSP_TASK
        , unsigned i_dsp
#endif
        ) __attribute__ ((weak));
void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[]
#ifdef RUN_DSP_TASK
        , unsigned i_dsp
#endif
)
{
    /* Do nothing */
}

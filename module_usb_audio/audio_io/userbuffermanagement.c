
#include "xccompat.h"
#include "devicedefines.h"

/* Default implentation for UserBufferManagement() */
void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[]
        , unsigned i_dsp
        ) __attribute__ ((weak));
void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[]
        , unsigned i_dsp
)
{
    /* Do nothing */
}

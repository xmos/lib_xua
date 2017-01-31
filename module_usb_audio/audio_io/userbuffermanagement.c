#include "userbuffermanagement.h"
#include "xccompat.h"
#include "devicedefines.h"

/* Default implementation for UserBufferManagementInit() */
void __attribute__ ((weak)) UserBufferManagementInit(CLIENT_INTERFACE(audManage_if, i_audMan))
{
    /* Do nothing */
}

/* Default implementation for UserBufferManagement() */
void __attribute__ ((weak)) UserBufferManagement(unsigned sampsFromUsbToAudio[],
                                                 unsigned sampsFromAudioToUsb[],
                                                 CLIENT_INTERFACE(audManage_if, i_audMan))
{
    /* Do nothing */
}

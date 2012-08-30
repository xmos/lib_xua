
#include "devicedefines.h"

#ifdef HOST_ACTIVE_CALL
void VendorHostActive(int valid);

void XUD_UserSuspend(void)
{
    VendorHostActive(0);
}

void XUD_UserResume(void)
{
    unsigned config;

    asm("ldw %0, dp[g_config]" : "=r" (config):);
    
    if(config == 1)
    {
        VendorHostActive(1);
    }
}
#endif

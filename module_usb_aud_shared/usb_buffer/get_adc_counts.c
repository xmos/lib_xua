extern unsigned int g_curUsbSpeed;
#define XUD_SPEED_HS 2
void GetADCCounts(unsigned samFreq, int *min, int *mid, int *max)
{
    unsigned frameTime;
    int usb_speed;
    usb_speed = g_curUsbSpeed;
    if (usb_speed == XUD_SPEED_HS) 
      frameTime = 8000;
    else 
      frameTime = 1000;

    *min = samFreq / frameTime;
    *max = *min + 1;

    *mid = *min;

    /* Check for INT(SampFreq/8000) == SampFreq/8000 */    
    if((samFreq % frameTime) == 0)
    {
        *min -= 1;
    }

}

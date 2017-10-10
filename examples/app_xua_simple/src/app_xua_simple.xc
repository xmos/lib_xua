
/* A very simple example of a USB audio application (and as such is un-verified)
 * 
 * It uses some tedious blocks from the lib_xua i.e. Endpoint 0, whilst re-implementing simple versions of others i.e. buffering 
 *
 */

#include <xs1.h>
#include <platform.h>

#include "xud_device.h"
#include "xua.h"


void OutBuffer(chanend c_aud_out)
{
    while(1)
    {

    }
}

void I2S()
{
    while(1)
    {

    }
}



/* Endpoint type tables - informs XUD what the transfer types for each Endpoint in use and also
 * if the endpoint wishes to be informed of USB bus resets
 */
XUD_EpType epTypeTableOut[] = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE};
XUD_EpType epTypeTableIn[] =   {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};

int main()
{
    /* Channels for lib_xud */
    chan c_ep_out[2];
    chan c_ep_in[1];

    chan c_aud_ctl;

    par
    {
        /* Low level USB device layer core */ 
        on tile[1]: XUD_Main(c_ep_out, 2, c_ep_in, 1,
                      null, epTypeTableOut, epTypeTableIn, 
                      null, null, -1 , XUD_SPEED_HS, XUD_PWR_BUS);
        
        /* Endpoint 0 core from lib_xua */
        /* Note, since we are not using many features we pass in null for quite a few params.. */
        on tile[1]: XUA_Endpoint0(c_ep_out[0], c_ep_in[0], c_aud_ctl, null, null, null, null);

        /* Our own simple buffering and I2S cores */
        on tile[1]: OutBuffer(c_ep_out[1]);

        /* Our own simple I2S driver core */
        on tile[1]: I2S();

    }
    return 0;
}



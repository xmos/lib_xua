#ifndef _VENDORREQUESTS_H_
#define _VENDORREQUESTS_H_

#include <xccompat.h>

/* Functions that handle vustomer vendor requests.
 *
 * THESE NEED IMPLEMENTING FOR A SPECIFIC DESIGN
 *
 * Should return 0 if handled sucessfully, else return 0 (-1 for passing up reset/suspend)
 *
 * */

int VendorAudioRequests(XUD_ep ep0_out, XUD_ep ep0_in, unsigned char bRequest, unsigned char cs, unsigned char cn,
    unsigned short unitId, unsigned char direction, chanend c_audioControl,
    NULLABLE_RESOURCE(chanend, c_mix_ctl),
    NULLABLE_RESOURCE(chanend, c_clk_ctL));

#endif


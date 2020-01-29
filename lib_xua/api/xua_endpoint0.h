// Copyright (c) 2011-2018, XMOS Ltd, All rights reserved

#ifndef _XUA_ENDPOINT0_H_
#define _XUA_ENDPOINT0_H_

#include "xua.h"
#include "dfu_interface.h"
#include "vendorrequests.h"

#if __XC__

/** Function implementing Endpoint 0 for enumeration, control and configuration
 *  of USB audio devices. It uses the descriptors defined in ``descriptors_2.h``.
 *
 *  \param c_ep0_out Chanend connected to the XUD_Manager() out endpoint array
 *  \param c_ep0_in Chanend connected to the XUD_Manager() in endpoint array
 *  \param c_audioCtrl Chanend connected to the decouple thread for control
 *                     audio (sample rate changes etc.)
 *  \param c_mix_ctl Optional chanend to be connected to the mixer thread if
 *                   present
 *  \param c_clk_ctl Optional chanend to be connected to the clockgen thread if
 *                   present.
 *  \param c_usb_test Optional chanend to be connected to XUD if test modes required.
 *
 *  \param c_EANativeTransport_ctrl Optional chanend to be connected to EA Native
 *                                  endpoint manager if present
 */
void XUA_Endpoint0(chanend c_ep0_out, chanend c_ep0_in, chanend c_audioCtrl,
        chanend ?c_mix_ctl,chanend ?c_clk_ctl, chanend ?c_EANativeTransport_ctr, client interface i_dfu ?dfuInterface
         VENDOR_REQUESTS_PARAMS_DEC_);

#endif
#endif

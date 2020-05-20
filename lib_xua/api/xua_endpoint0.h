// Copyright (c) 2011-2020, XMOS Ltd, All rights reserved

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

/** Function to set the Vendor ID value
 *
 *  \param vid vendor ID value to set
*/
void XUA_Endpoint0_setVendorId(unsigned short vid);

/** Function to set the Product ID value
 *
 *  \param pid Product ID value to set
*/
void XUA_Endpoint0_setProductId(unsigned short pid);


/** Function to set the Vendor string
 *
 *  \param vendor_str Vendor string to set
*/
#ifdef __XC__
void XUA_Endpoint0_setVendorStr(char * unsafe vendor_str);
#else
void XUA_Endpoint0_setVendorStr(char * vendor_str);
#endif

/** Function to set the Product string
 *
 *  \param product_str Product string to set
*/

#ifdef __XC__
void XUA_Endpoint0_setProductStr(char * unsafe product_str);
#else
void XUA_Endpoint0_setProductStr(char * product_str);
#endif

/** Function to set the Serial string
 *
 *  \param serial_str Serial string to set
*/
#ifdef __XC__
void XUA_Endpoint0_setSerialStr(char * unsafe serial_str);
#else
void XUA_Endpoint0_setSerialStr(char * serial_str);
#endif

/** Function to set the BCD device
 *
 *  \param bcdDevice BCD device to set

*/

void XUA_Endpoint0_setBcdDevice(unsigned short bcdDevice);

/** Function to get the Vendor string
 *
 *  \return vendor string
*/
#ifdef __XC__
char * unsafe XUA_Endpoint0_getVendorStr();
#else
char *  XUA_Endpoint0_getVendorStr();
#endif

/** Function to get the Product string
 *
 *  \return Product string
*/
#ifdef __XC__
char * unsafe XUA_Endpoint0_getProductStr();
#else
char *  XUA_Endpoint0_getProductStr();
#endif

/** Function to get the Serial Number string
 *
 *  \return Serial string
*/
#ifdef __XC__
char * unsafe XUA_Endpoint0_getSerialStr();
#else
char *  XUA_Endpoint0_getSerialStr();
#endif

/** Function to get the Vendor ID
 *
 *  \return Vendor ID
*/
unsigned short XUA_Endpoint0_getVendorId();

/** Function to get the Product ID
 *
 *  \return Product ID
*/
unsigned short XUA_Endpoint0_getProductId();

/** Function to get the BCD device
 *
 *  \return BCD device
*/
unsigned short XUA_Endpoint0_getBcdDevice();

#endif
#endif

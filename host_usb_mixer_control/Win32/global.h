/**
 * Module:  host_usb_mixer_control
 * Version: 1v0
 * Build:   d94b0511afe40ece896637f88c6379f9b6f9f603
 * File:    global.h
 *
 * The copyrights, all other intellectual and industrial 
 * property rights are retained by XMOS and/or its licensors. 
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2010
 *
 * In the case where this code is a modification of existing code
 * under a separate license, the separate license terms are shown
 * below. The modifications to the code are still covered by the 
 * copyright notice above.
 *
 **/                                   
/************************************************************************
 *
 *  Module:       global.h
 *  Description:
 *     APP global includes, constants, declarations, etc.
 *
 *  Author(s):   
 *    Udo Eberhardt
 *                
 *  Companies:
 *    Thesycon GmbH, Germany      http://www.thesycon.de
 *                
 ************************************************************************/

#ifndef __global_h__
#define __global_h__

// define the Windows versions supported by the application
#define _WIN32_WINNT 0x0500     //Windows 2000 or later
//#define _WIN32_WINNT 0x0501     //Windows XP or later
//#define _WIN32_WINNT 0x0600     //Windows Vista or later

// exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <stdio.h>
#include <tchar.h>


// version defs
//#include "version.h"

// libwn.h pulls in windows.h
#include "libwn.h"
// TUSBAUDIO driver API
#include "tusbaudioapi.h"
#include "TUsbAudioApiDll.h"


#endif  // __global_h__

/*************************** EOF **************************************/

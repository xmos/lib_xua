// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

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
//#define _WIN32_WINNT 0x0A00     //Windows 10 or later

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

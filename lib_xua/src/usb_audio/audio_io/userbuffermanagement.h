#ifndef _USERBUFFERMANAGEMENT_H_
#define _USERBUFFERMANAGEMENT_H_

#include "xua_audio.h"
#include <xccompat.h>

void UserBufferManagementInit(CLIENT_INTERFACE(audManage_if, i_audMan));

void UserBufferManagement(unsigned sampsFromUsbToAudio[],
                          unsigned sampsFromAudioToUsb[],
                          CLIENT_INTERFACE(audManage_if, i_audMan));

#endif // _USERBUFFERMANAGEMENT_H_

// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __requests_h__
#define __requests_h__

#include <xccompat.h>

void initialise(void);
void download(CLIENT_INTERFACE(i_dfu, i), const char file_name[]);
void revertfactory(CLIENT_INTERFACE(i_dfu, i));

#endif

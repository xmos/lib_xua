// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
//#pragma select handler
#include <xs1.h>

/* TODO Currently complier does not support inline select functions, hense this is in a seperate file to ensure this is not the case */
#pragma select handler
static inline void testct_byref(chanend c, unsigned &isCt)
{
    if (testct(c))
    {
        isCt = 1;
    }
    else
    {
        isCt = 0;
    }
}
//void testct_byref(chanend c, unsigned &isCt) ;

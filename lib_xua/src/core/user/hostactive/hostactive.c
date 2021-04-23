// Copyright 2013-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

void UserHostActive(int active) __attribute__ ((weak));
void UserHostActive(int active)
{
    return;
}


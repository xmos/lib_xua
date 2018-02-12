// Copyright (c) 2013-2018, XMOS Ltd, All rights reserved

void UserHostActive(int active) __attribute__ ((weak));
void UserHostActive(int active)
{
    return;
}


// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* Default implementations of XUA_UserSuspendPowerDown(), XUA_UserSuspendPowerUp() */

void XUA_UserSuspendPowerDown() __attribute__ ((weak));
void XUA_UserSuspendPowerDown()
{
    return;
}

void XUA_UserSuspendPowerUp() __attribute__ ((weak));
void XUA_UserSuspendPowerUp()
{
    return;
}

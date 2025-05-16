// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* Default implementations of AudioHwSuspendPowerDown(), AudioHwSuspendPowerUp() */


void SuspendPowerDown() __attribute__ ((weak));
void SuspendPowerDown()
{
    return;
}

void SuspendPowerUp() __attribute__ ((weak));
void SuspendPowerUp()
{
    return;
}
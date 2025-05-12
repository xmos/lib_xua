// Copyright 2013-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* Default implementation of UserAudioStreamState() callback.
 */


void UserAudioStreamState(int inputActive, int outputActive) __attribute__ ((weak));
void UserAudioStreamState(int inputActive, int outputActive)
{

}
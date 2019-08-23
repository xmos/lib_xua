// Copyright (c) 2013-2018, XMOS Ltd, All rights reserved

/* Default implementations of AudioStreamStop() and AudioStreamStart()
 * callbacks.
 */

void UserAudioStreamStop() __attribute__ ((weak));
void UserAudioStreamStop()
{
    return;
}

void UserAudioStreamStart() __attribute__ ((weak));
void UserAudioStreamStart()
{
    return;
}

void UserAudioInputStreamStop() __attribute__ ((weak));
void UserAudioInputStreamStop()
{
    return;
}

void UserAudioInputStreamStart() __attribute__ ((weak));
void UserAudioInputStreamStart()
{
    return;
}

void UserAudioOutputStreamStop() __attribute__ ((weak));
void UserAudioOutputStreamStop()
{
    return;
}

void UserAudioOutputStreamStart() __attribute__ ((weak));
void UserAudioOutputStreamStart()
{
    return;
}

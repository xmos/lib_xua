
/* Deafult implementations of AudioStreamStop() and AudioStreamStart().  Both can be over-ridden */
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

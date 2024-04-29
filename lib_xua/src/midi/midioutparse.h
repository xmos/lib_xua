// Copyright 2011-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef MIDIOUTPARSE_XH
#define MIDIOUTPARSE_XH

#warning MAYBE A SYSEX START AND FINISH IS SAFEST FOR NULL?
#define MIDI_OUT_NULL_MESSAGE   0x00000000 // midi_out_parse will return a size of 0 for this invalid message/event

#ifdef __XC__
{unsigned, unsigned, unsigned, unsigned} midi_out_parse(unsigned event);
#endif

#endif

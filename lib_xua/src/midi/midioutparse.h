// Copyright 2011-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef MIDIOUTPARSE_XH
#define MIDIOUTPARSE_XH

// If for any reason we pop a message when not needed (should never happen) this will cause midiparse out to send a size of 0 (drops packet)
#define MIDI_OUT_NULL_MESSAGE   0x00000000

#ifdef __XC__
// Takes a MIDI packet and decomoses it into up to 3 data bytes followed by a byte count.
{unsigned, unsigned, unsigned, unsigned} midi_out_parse(unsigned event);
#endif

#endif

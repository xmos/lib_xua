// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef MIDIINPARSE_XH
#define MIDIINPARSE_XH

#define INITIAL 0
#define INCHANNEL_MSG 1
#define INSYSCOMMON_MSG 2
#define INSYSEX_MSG 3

struct midi_in_parse_state {
    // State for the parser
    unsigned expect_msg_len;
    unsigned msg_type;

    unsigned receivebuffer[3];
    unsigned received;

    unsigned codeIndexNumber;
};

void dump_midi_in_parse_state(struct midi_in_parse_state &s);
void reset_midi_state(struct midi_in_parse_state &mips);
{unsigned int , unsigned int} midi_in_parse(struct midi_in_parse_state &mips, unsigned cable_number, unsigned char b);

#endif

// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
/**
 * @file midiinparse.xc
 * @brief Generates USB MIDI events from MIDI events
 * @author Russell Gallop, XMOS Semiconductor
 */

#include <print.h>
#include "midiinparse.h"

/**
 * @brief Report state of the MIDI in parser (should be removed by deadcode elimination)
 *
 */
void dump_midi_in_parse_state(struct midi_in_parse_state &s) {
    printstr("expect_msg_len: 0x"); printhexln(s.expect_msg_len);
    printstr("msg_type: 0x"); printhexln(s.msg_type);
    printstr("receivebuffer: 0x"); printhex(s.receivebuffer[0]);
    printstr(", 0x"); printhex(s.receivebuffer[1]);
    printstr(", 0x"); printhexln(s.receivebuffer[2]);
    printstr("received: 0x"); printhexln(s.received);
    printstr("codeIndexNumber: 0x"); printhexln(s.codeIndexNumber);
}

/**
 * @brief Reset state of MIDI parser
 *
 */
void reset_midi_state(struct midi_in_parse_state &mips) {
    mips.expect_msg_len = 0;
    mips.msg_type = 0;

    mips.receivebuffer[0] = 0;
    mips.receivebuffer[1] = 0;
    mips.receivebuffer[2] = 0;
    mips.received = 0;
    mips.codeIndexNumber = 0;
}

/**
 * @brief Construct USB MIDI event
 *
 */
static unsigned makeEvent(unsigned cable_number, unsigned codeIndexNumber, unsigned midi0, unsigned midi1, unsigned midi2) {
    unsigned event = (cable_number << 28);
    event |= (codeIndexNumber << 24);
    event |= (midi0 << 16);
    event |= (midi1 << 8);
    event |= (midi2 << 0);

    return event;
}

/**
 * @ brief MIDI input parser
 *
 */
{unsigned int , unsigned int} midi_in_parse(struct midi_in_parse_state &state, unsigned cable_number, unsigned char b) {
    unsigned valid = 0;
    unsigned data = 0xBADDF00D; // should never be returned along with valid = 1

    unsigned highNibble = (b & 0xF0) >> 4;
    unsigned lowNibble = (b & 0xF);

/*
    assert(!(state.expect_msg_len && state.msg_type == INSYSEX_MSG));
    assert((state.received >= 1) || (state.receivebuffer[0] == 0));
    assert((state.received >= 2) || (state.receivebuffer[1] == 0));
    assert((state.received == 3) || (state.receivebuffer[2] == 0));
    assert(state.received < 3);
*/

    if (b & 0x80) { // Is status byte
        if (highNibble == 0xF) { // System message
            if (lowNibble & 0x8) { // System real time
                // System Real-Time Messages (can interleave system exclusive and between header and data (page 30 of complete MIDI spec))
                //case 0x8: // Timing tick
                //case 0x9: // Reserved
                //case 0xA: // Start song
                //case 0xB: // Continue song
                //case 0xC: // Stop song
                //case 0xD: // Reserved
                //case 0xE: // Active sensing
                //case 0xF: // System reset
                // Have complete event, send out
                valid = 1;
                data = makeEvent(cable_number, highNibble, b, 0, 0);
            } else {
                if (b == 0xF7) { // End of SysEx
                    state.receivebuffer[state.received] = b;
                    state.received++;
                    // Compose sysex bytes that we've got and send them out.
                    // This will depend how many we have.
                    state.codeIndexNumber = state.received + 0x4;
                    valid = 1;
                    data = makeEvent(cable_number, state.codeIndexNumber,
                                     state.receivebuffer[0],  state.receivebuffer[1],  state.receivebuffer[2]);
                    reset_midi_state(state);
                } else {
                    reset_midi_state(state);
                    state.receivebuffer[state.received] = b;
                    state.received++;
                    switch (lowNibble)
                    {
                    case 0x2: // Song Position Pointer (3 byte system common)
                    {
                        state.msg_type = INSYSCOMMON_MSG;
                        state.expect_msg_len = 3;
                        state.codeIndexNumber = 3;
                        break;
                    }
                    case 0x1: // MIDI Time Code (2 byte system common)
                    case 0x3: // Song Select (2 byte system common)
                    {
                        state.msg_type = INSYSCOMMON_MSG;
                        state.expect_msg_len = 2;
                        state.codeIndexNumber = 2;
                        break;
                    }
                    case 0x6: // Tune request (1 byte system common)
                        state.codeIndexNumber = 5;
                        valid = 1;
                        data = makeEvent(cable_number, state.codeIndexNumber,
                                         state.receivebuffer[0],  state.receivebuffer[1],  state.receivebuffer[2]);
                        break;
                    case 0x0: // Sysex start byte, never send based on just this
                        state.msg_type = INSYSEX_MSG;
                        break;
                    default:
                        // Could happen with unrecognised headers, e.g. 0xF4, 0xF5
                        // Just pass on
                        valid = 1;
                        data = makeEvent(cable_number, 0x0f, b, 0, 0);
                        reset_midi_state(state);
                        break;
                    }
                }
            }
        } else { // Channel message
            reset_midi_state(state);
            state.receivebuffer[state.received] = b;
            state.received++;
            // code index number is always the high nibble for channel messages
            state.codeIndexNumber = highNibble;
            switch (highNibble)
            {
            case 0x8: // Note-off
            case 0x9: // Note-on
            case 0xA: // Poly-KeyPress
            case 0xB: // Control Change
            case 0xE: // PitchBend Change
            {
                state.msg_type = INCHANNEL_MSG;
                state.expect_msg_len = 3;
                break;
            }
            case 0xC: // Program Change
            case 0xD: // Channel Pressure
            {
                state.msg_type = INCHANNEL_MSG;
                state.expect_msg_len = 2;
                break;
            }
            }
        }
    } else { // data byte
        state.receivebuffer[state.received] = b;
        state.received++;
        switch (state.msg_type) {
        case INCHANNEL_MSG:
        case INSYSCOMMON_MSG:
        {
            if (state.received == state.expect_msg_len) {
                valid = 1;
                data = makeEvent(cable_number, state.codeIndexNumber,
                                 state.receivebuffer[0], state.receivebuffer[1], state.receivebuffer[2]);
                if (state.msg_type == INSYSCOMMON_MSG) {
                    // No running status on system common messages
                    reset_midi_state(state);
                } else {
                    // Keep the first byte on channel messages, already received 1 byte
                    state.received = 1;
                    state.receivebuffer[1] = 0;
                    state.receivebuffer[2] = 0;
                }
            }
            break;
        }
        case INSYSEX_MSG:
        {
            if ((state.received == 3)) {
                // Output if have 3 using the SysEx starts or continues
                state.codeIndexNumber = 0x4;
                valid = 1;
                data = makeEvent(cable_number, state.codeIndexNumber,
                                 state.receivebuffer[0],  state.receivebuffer[1],  state.receivebuffer[2]);
                // reset buffer but not msg_type
                state.received = 0;
                state.receivebuffer[0] = 0;
                state.receivebuffer[1] = 0;
                state.receivebuffer[2] = 0;
            }
            break;
        }
        default:
        {
            // Else data byte with no status so just send as single byte without parsing.
            valid = 1;
            data = makeEvent(cable_number, 0x0f, b, 0, 0);
            reset_midi_state(state);
            break;
        }
        }
    }

    return {valid, data};
}

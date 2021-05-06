// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
/**
 * @file midioutparse.xc
 * @brief Parses USB-MIDI events into set of MIDI bytes
 * @author Russell Gallop, XMOS Semiconductor
 */

#include "midioutparse.h"

/**
 * @brief Breaks a USB-MIDI event into it's constituant fields
 *
 * @param[in]   ev    USB-MIDI event
 */
#if 1
{unsigned, unsigned, unsigned, unsigned, unsigned} static breakEvent(unsigned ev) {
    unsigned cable_number = (ev >> 28) & 0xf;
    unsigned codeIndexNumber = (ev >> 24) & 0xf;
    unsigned midi0 = (ev >> 16) & 0xff;
    unsigned midi1 = (ev >> 8) & 0xff;
    unsigned midi2 = (ev >> 0) & 0xff;
    return {cable_number, codeIndexNumber, midi0, midi1, midi2};
}
#endif


/**
 * @brief Parse a USB-MIDI event into the MIDI bytes and a length field
 *
 * @param[in]   ev    USB-MIDI event
 */
{unsigned, unsigned, unsigned, unsigned} midi_out_parse(unsigned event) {
    unsigned cable_number; // ignore this for now!
    unsigned codeIndexNumber;
    unsigned midi[3];
    unsigned size = 0;

    {cable_number, codeIndexNumber, midi[0], midi[1], midi[2]} = breakEvent(event);

    // Not doing anything with cable number
    switch (codeIndexNumber) {
    case 0x3: // Three-byte system Common messages like SPP, etc.
    case 0x4: // SysEx starts or continues
    case 0x7: // SysEx ends with the following three bytes
    case 0x8: // Note-off
    case 0x9: // Note-on
    case 0xA: // Poly-KeyPress
    case 0xB: // Control Change
    case 0xE: // PitchBend Change
    {
        size = 3;
        break;
    }
    case 0x2: // Two-byte system Common messages like MTC, SongSelect, etc.
    case 0x6: // SysEx ends with the following two bytes
    case 0xC: // Program Change
    case 0xD: // Channel Pressure
    {
        size = 2;
        break;
    }
    case 0x5: // Single-byte System Common Message or SysEx ends with following single byte.
    case 0xF: // Single byte
    {
        size = 1;
        break;
    }
    default:
        break;
    }
    return {midi[0], midi[1], midi[2], size};
}

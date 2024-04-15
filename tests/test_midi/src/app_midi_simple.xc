// Copyright 2017-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* A very simple *example* of a USB audio application (and as such is un-verified for production)
 *
 * It uses the main blocks from the lib_xua
 *
 * - 2 channels out I2S only
 * - No DFU
 * - I2S only
 *
 */

#include <xs1.h>
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <xclib.h>

#include "xua.h"
#include "xud_device.h"
#include "midiinparse.h"
#include "midioutparse.h"

on tile[MIDI_TILE] :  port p_midi_tx                        = XS1_PORT_4C;
#if(MIDI_RX_PORT_WIDTH == 4)
on tile[MIDI_TILE] :  buffered in port:4 p_midi_rx          = XS1_PORT_1F;
#elif(MIDI_RX_PORT_WIDTH == 1)
on tile[MIDI_TILE] :  buffered in port:1 p_midi_rx          = XS1_PORT_1F;
#endif
#define CLKBLK_MIDI  XS1_CLKBLK_2
on tile[MIDI_TILE] : clock    clk_midi                      = CLKBLK_MIDI;


/* Port declarations for I2C to config ADC's */
on tile[0]: port p_scl = XS1_PORT_1L;
on tile[0]: port p_sda = XS1_PORT_1M;

/* See hwsupport.xc */
void ctrlPort();

#define CABLE_NUM   0

#define NOTE_ON     0x90
#define PITCH       60
#define VELOCITY    80

void test(chanend c_midi){
    struct midi_in_parse_state mips;
    reset_midi_state(mips);

    unsigned valid = 0;
    unsigned tx_data = 0;
    {valid, tx_data} = midi_in_parse(mips,CABLE_NUM, NOTE_ON);
    printf("Valid: %d data: %u\n", valid, tx_data);
    {valid, tx_data} = midi_in_parse(mips, CABLE_NUM, PITCH);
    printf("Valid: %d data: %u\n", valid, tx_data);
    {valid, tx_data} = midi_in_parse(mips, CABLE_NUM, VELOCITY);
    printf("Valid: %d data: %u\n", valid, tx_data);


    char midi[3];
    unsigned size = 0;
    {midi[0], midi[1], midi[2], size} = midi_out_parse(tx_data);
    printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi[0], midi[1], midi[2]);


    int is_ack;
    unsigned int datum;

    unsigned count = 0;
    while(1){
        select{
            case midi_get_ack_or_data(c_midi, is_ack, datum):
                printf("ACK: %d Datum: 0x%x\n", is_ack, datum);
                count++;
                if(count == 3){
                    delay_microseconds(200); // Allow frame to complete
                    exit(0);
                }
            break;

            default:
                outuint(c_midi,  byterev(tx_data));
                printf("SEND TO MIDI\n");
                delay_milliseconds(2); // 30 bits at 31.25 kbps is 0.96ms
            break;
        }
    }

}


int main()
{
    chan c_midi;

 
    par
    {
        on tile[0]: test(c_midi);
        on tile[1]: usb_midi(p_midi_rx, p_midi_tx, clk_midi, c_midi, 0);

        // Setup HW so we can run this on the MC board 
        on tile[0]: ctrlPort();
    }

    return 0;
}



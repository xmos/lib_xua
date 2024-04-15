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
#include <stdint.h>
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

#define MAX_TEST_COMMANDS   10
#define TEST_COMMAND_FILE   "midi_tx_cmds.txt"

/* See hwsupport.xc */
void ctrlPort();

#define CABLE_NUM   0


unsigned mini_in_parse_helper(unsigned midi[3]){
    // printf("Composing data: 0x%x 0x%x 0x%x\n", midi[0], midi[1], midi[2]);

    struct midi_in_parse_state m_state;
    reset_midi_state(m_state);

    unsigned valid = 0;
    unsigned packed = 0;

    for(int i = 0; i < 3; i++){
        {valid, packed} = midi_in_parse(m_state, CABLE_NUM, midi[i]);
        if(valid){
            return packed;
        }
    }
    return 0;
}

unsigned parse_cmd_line(uint8_t commands[MAX_TEST_COMMANDS][3])
{
    FILE * movable fptr_tx = fopen(TEST_COMMAND_FILE,"rt");
    if (fptr_tx == NULL) {
        printf("ERROR: TX command file %s not found or unable to open.\n", TEST_COMMAND_FILE);
        fclose(move(fptr_tx));
        return 0;
    }

    unsigned line = 0;

    unsigned a,b,c;
    while (fscanf(fptr_tx, "%u %u %u\n", &a, &b, &c) == 3) {
        commands[line][0] = a;
        commands[line][1] = b;
        commands[line][2] = c;
        // printf("Line %u params: 0x%x 0x%x 0x%x\n", line, commands[line][0], commands[line][1], commands[line][2]);
        line++;
        if(line > MAX_TEST_COMMANDS){
            printf("ERROR: Too many lines in TX command file\n");

            fclose(move(fptr_tx));
            return MAX_TEST_COMMANDS;
        }
    }

    fclose(move(fptr_tx));
    return line;
}

void test(chanend c_midi){
    uint8_t commands[MAX_TEST_COMMANDS][3] = {{0}};
    unsigned num_tx = parse_cmd_line(commands);

    int is_ack;
    unsigned datum;

    unsigned line = 0;
    while(1){
        select{
            case midi_get_ack_or_data(c_midi, is_ack, datum):
                // printf("ACK: %d Datum: 0x%x\n", is_ack, datum);
                line++;
                if(line == num_tx){
                    delay_microseconds(200); // Allow frame to complete
                    exit(0);
                }
            break;

            default:
                if(num_tx){
                    unsigned midi[] = {commands[line][0], commands[line][1], commands[line][2]};
                    unsigned tx_packet = mini_in_parse_helper(midi);
                    outuint(c_midi,  byterev(tx_packet));
                    // printf("SEND TO MIDI: 0x%x\n", tx_packet);
                    delay_milliseconds(2); // 30 bits at 31.25 kbps is 0.96ms
                } else {
                    exit(0);
                }
            break;
        }
    }

}


int main(void)
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



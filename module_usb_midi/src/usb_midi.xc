#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include <stdint.h>
#include "usb_midi.h"
#include "midiinparse.h"
#include "midioutparse.h"
#include "queue.h"
#ifdef IAP
#include "iAP.h"
#include "iapuser.h"
#endif
//#define MIDI_LOOPBACK 1
int icount = 0;
static unsigned makeSymbol(unsigned data)
{
    // Start and stop bits to the data packet
    //  like 10'b1dddddddd0
    return (data << 1) | 0x200;
}

#define RATE 31250

#ifndef MIDI_SHIFT_TX
#define MIDI_SHIFT_TX 7
#endif

static unsigned bit_time =  XS1_TIMER_MHZ * 1000000 / (unsigned) RATE;
static unsigned bit_time_2 =  (XS1_TIMER_MHZ * 1000000 / (unsigned) RATE) / 2;

// For debugging
int mr_count = 0; // MIDI received (from HOST)
int th_count = 0; // MIDI sent (To Host)

int uout_count = 0; // UART bytes out
int uin_count = 0; // UART bytes in

// state for iAP
#ifdef IAP
extern unsigned authenticating;
#else
unsigned authenticating = 0;
#endif

// state for auto-selecting dock or USB B
extern unsigned polltime;

#ifdef IAP
timer iAPTimer;
#endif

void usb_midi(port ?p_midi_in, port ?p_midi_out,
            clock ?clk_midi,
            chanend ?c_midi,
            unsigned cable_number,
            chanend ?c_iap, chanend ?c_i2c, // iOS stuff
            port ?p_scl, port ?p_sda
)
{
    unsigned symbol = 0x0; // Symbol in progress of being sent out
    unsigned isTX = 0; // Guard when outputting data
    unsigned txT; // Timer value used for outputting
    //unsigned inputPortState, newInputPortState;
    int waiting_for_ack = 0;
    // Receiver
    unsigned rxByte;
    int rxI;
    int rxT;
    int isRX = 0; // Guard when receiving data
    timer t;
    timer t2;

    // One place buffer for data going out to host
    queue midi_to_host_fifo;
    unsigned char midi_to_host_fifo_arr[4]; // Used for 32bit USB MIDI events

    unsigned outputting_symbol, outputted_symbol;

    struct midi_in_parse_state mips;

    // the symbol fifo (to go out of uart)
    queue symbol_fifo;
    unsigned char symbol_fifo_arr[USB_MIDI_DEVICE_OUT_FIFO_SIZE * 4]; // Used for 32bit USB MIDI events

    unsigned rxPT, txPT;
    int midi_from_host_overflow = 0;

    //configure_clock_rate(clk_midi, 100, 1);
    init_queue(symbol_fifo, symbol_fifo_arr, USB_MIDI_DEVICE_OUT_FIFO_SIZE, 4);
    init_queue(midi_to_host_fifo, midi_to_host_fifo_arr, 1, 4);

    configure_out_port(p_midi_out, clk_midi, 1<<MIDI_SHIFT_TX);
    configure_in_port(p_midi_in, clk_midi);

    /* Just in case not using CLKBLK_REF */
    start_clock(clk_midi);

    reset_midi_state(mips);

    t :> txT;
    t2 :> rxT;

#ifdef IAP
    CoProcessorDisable();
#endif
      
   // p_midi_out <: 1 << MIDI_SHIFT_TX; // Start with high bit.
#ifdef IAP  
    CoProcessorEnable();
#endif

#ifdef IAP
    /* Check for special case where MIDI and i2c ports are shared... */
    if(isnull(c_i2c) && isnull(p_scl) && isnull(p_sda))
    {
        init_iAP(c_i2c, p_midi_out, p_midi_in); // uses timer for i2c initialisation pause..
    }
    else
    {   
        init_iAP(c_i2c, p_scl, p_sda); // uses timer for i2c initialisation pause..
    }
#endif

    {
#ifdef IAP
        iAPTimer :> polltime;
        polltime += XS1_TIMER_HZ / 2;
#endif
        while (1) 
        {
            int is_ack;
            int is_reset;
            unsigned int datum;
            select 
            {
                // Input to read the start bit
#ifndef MIDI_LOOPBACK
                case (!authenticating && !isRX) => p_midi_in when pinseq(0) :> void @  rxPT:
                    isRX = 1;
                    t2 :> rxT;
                    rxT += (bit_time + bit_time_2);
                    rxPT += (bit_time + bit_time_2); // absorb start bit and set to halfway through the next bit
                    rxI = 0;
                    asm("setc res[%0],1"::"r"(p_midi_in));
                    asm("setpt res[%0],%1"::"r"(p_midi_in),"r"(rxPT));
                    break;
        
                // Input to read the remaining bits
                case (!authenticating && isRX) => t2 when timerafter(rxT) :> int _ :
                {
                    unsigned bit;
                    p_midi_in :> bit;
                    if (rxI++ < 8) 
                    {
                        // shift in bits into the high end of a word
                        rxByte = (bit << 31) | (rxByte >> 1);
                        rxT += bit_time;
                        rxPT += bit_time;
                        asm("setpt res[%0],%1"::"r"(p_midi_in),"r"(rxPT));
                    } 
                    else 
                    {
                        // rcv and check stop bit
                        if ((bit & 0x1) == 1) 
                        {
                            unsigned valid = 0;
                            unsigned event = 0;
                            uin_count++;
                            rxByte >>= 24;
                            //                if (rxByte != outputted_symbol) {
                            //                  // Loopback check
                            //                  printhexln(rxByte);
                            //                  printhexln(outputted_symbol);
                            //                }

                            {valid, event} = midi_in_parse(mips, cable_number, rxByte);
                            if (valid && isempty(midi_to_host_fifo)) 
                            {

                                event = byterev(event);
                                // data to send to host - add to fifo
                                if (!waiting_for_ack) 
                                {
                                    // send data
                                    // printstr("uart->decouple: ");
                                    outuint(c_midi, event);
                                    waiting_for_ack = 1;
                                    th_count++;
                                } 
                                else 
                                {
                                    enqueue(midi_to_host_fifo, event);
                                }
                            } 
                            else if (valid) 
                            {
                                //printstr("g");
                            }
                        }
                    isRX = 0;
                }
                break;
            }

        // Output
        // If isTX then feed the bits out one at a time
        //  until symbol is zero expect pattern like 10'b1dddddddd0
        // This code will leave the output high afterwards due to the stop bit added with makeSymbol
        case (!authenticating && isTX) => t when timerafter(txT) :> int _:
            if (symbol == 0) 
            {
                // Got something to output but not mid-symbol.
                // Start sending symbol.
                //  This case is reached when a symbol has been received from the host but not started AND
                //  When it has just finished sending a symbol

                // Take from FIFO
                outputting_symbol = dequeue(symbol_fifo);
                symbol = makeSymbol(outputting_symbol);

                if (space(symbol_fifo) > 3 && midi_from_host_overflow) 
                {
                    midi_from_host_overflow = 0;
                    midi_send_ack(c_midi);
                }

                p_midi_out <: (1<<MIDI_SHIFT_TX) @ txPT;
                //              printstr("mout1\n");
                t :> txT;
                txT += bit_time;
                txPT += bit_time;
                isTX = 1;
            } 
            else 
            {
                // Mid-symbol
                txT += bit_time; // Should this be after the output otherwise be double the length of the high before the start bit
                txPT += bit_time;
                p_midi_out @ txPT <: ((symbol & 1)<<MIDI_SHIFT_TX);
                //            printstr("mout2\n");
                symbol >>= 1;
                if (symbol == 0) 
                {
                    // Finished sending byte
                    uout_count++;
                    outputted_symbol = outputting_symbol;
                    if (isempty(symbol_fifo)) 
                    { // FIFO empty
                        isTX = 0;
                    }
                }
            }
            break;
#endif

        case !authenticating => midi_get_ack_or_data(c_midi, is_ack, datum):

            if (is_ack) 
            {
                // have we got more data to send
                //printstr("ack\n");
                if (!isempty(midi_to_host_fifo)) 
                {
                    //printstr("uart->decouple\n");
                    outuint(c_midi, dequeue(midi_to_host_fifo));
                    th_count++;
                } 
                else 
                {
                    waiting_for_ack = 0;
                }
            } 
            else 
            {
                unsigned midi[3];
                unsigned size;
                // received data from host
                int event = byterev(datum);
                mr_count++;
#ifdef MIDI_LOOPBACK
                if (isempty(midi_to_host_fifo)) 
                {
                    // data to send to host
                    if (!waiting_for_ack) 
                    {
                        // send data
                        event = byterev(event);
                        outuint(c_midi, event);
                        th_count++;
                        waiting_for_ack = 1;
                    } 
                    else 
                    {
                        event = byterev(event);
                        enqueue(midi_to_host_fifo, event);
                    }
                    midi_send_ack(c_midi);
                }
                else
                {
                    //printstr("DROP\n");
                }
#else
                {midi[0], midi[1], midi[2], size} = midi_out_parse(event);
                for (int i = 0; i != size; i++) 
                {
                    // add symbol to fifo
                    enqueue(symbol_fifo, midi[i]);
                }

                if (space(symbol_fifo) > 3) 
                {
                    midi_send_ack(c_midi);
                } 
                else 
                {
                    midi_from_host_overflow = 1;
                }
                // Drop through to the isTX guarded case
                if (!isTX) 
                {
                    t :> txT; // Should be enough to trigger the other case
                    isTX = 1;
                }
#endif
            }
            break;
#ifdef IAP
                case !(isTX || isRX) => iap_get_ack_or_reset_or_data(c_iap, is_ack, is_reset, datum):

                    /* Check for special case where MIDI ports are shared with i2c ports */
                    if(isnull(c_i2c) && isnull(p_scl) && isnull(p_sda))
                    {
                        handle_iap_case(is_ack, is_reset, datum, c_iap, c_i2c, p_midi_out, p_midi_in);
                    }
                    else
                    {
                        handle_iap_case(is_ack, is_reset, datum, c_iap, c_i2c, p_scl, p_sda);
                    }
                    if (!authenticating) 
                    {
                        // printstrln("Completed authentication");
                        p_midi_in :> void; // Change port around to input again after authenticating (unique to midi+iAP case)
                    }
                    break;

                /* Slow timer looking for IDevice plug/unplug event */
                case iAPTimer when timerafter(polltime) :> void:
                    printintln(polltime);
                    handle_poll_dev_det(iAPTimer);
                    break;
#endif
            }
        }
    }
}


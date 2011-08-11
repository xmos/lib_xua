#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include "usb_midi.h"
#include "midiinparse.h"
#include "midioutparse.h"
#include "queue.h"

//#define MIDI_LOOPBACK 1

static unsigned makeSymbol(unsigned data) {
    // Start and stop bits to the data packet
    return (data << 1) | 0x200;
}

#define RATE 31250

static unsigned bit_time =  XS1_TIMER_MHZ * 1000000 / (unsigned) RATE;
static unsigned bit_time_2 =  (XS1_TIMER_MHZ * 1000000 / (unsigned) RATE) / 2;

// For debugging
int mr_count = 0; // MIDI received (from HOST)
int th_count = 0; // MIDI sent (To Host)

#ifdef MIDI_LOOPBACK
static inline void handle_byte_from_uart(chanend c_midi,   struct midi_in_parse_state &mips, int cable_number, 
                                         int &got_next_event, int &next_event, int &waiting_for_ack, int byte)
{
  int valid;
  unsigned event;
  {valid, event} = midi_in_parse(mips, cable_number, byte);          
  if (valid && !got_next_event) {
    // data to send to host
    if (!waiting_for_ack) {
      // send data         
      event = byterev(event);                   
      outuint(c_midi, event);
      th_count++;
      waiting_for_ack = 1;
    }
    else {
      event = byterev(event);
      next_event = event;
      got_next_event = 1;
    }
  }
  else if (valid) {
    // printstr("g\n");
  }
}
#endif

int uout_count = 0; // UART bytes out
int uin_count = 0; // UART bytes in 

void usb_midi(in port ?p_midi_in, out port ?p_midi_out, 
              clock ?clk_midi,
              chanend c_midi,
              unsigned cable_number)
{
  unsigned symbol = 0x0; // Symbol in progress of being sent out
  unsigned outputting = 0; // Guard when outputting data
  unsigned time; // Timer value used for outputting
  //unsigned inputPortState, newInputPortState;
  int waiting_for_ack = 0;
  // Receiver
  unsigned rxByte;
  int rxI;
  int rxT;
  int isRX = 0;
  timer t;
  timer t2;

  // these two vars make a one place buffer for data going out to host
  int got_next_event = 0;
  int next_event;
  unsigned outputting_symbol, outputted_symbol;

  struct midi_in_parse_state mips;

  // the symbol fifo (to go out of uart)
  queue q;
  unsigned symbol_fifo[USB_MIDI_DEVICE_OUT_FIFO_SIZE];

  unsigned rxPT, txPT;
  int midi_from_host_overflow = 0;

  //configure_clock_rate(clk_midi, 100, 1);
  init_queue(q, symbol_fifo, USB_MIDI_DEVICE_OUT_FIFO_SIZE);
  
  configure_out_port_no_ready(p_midi_out, clk_midi, 1);
  configure_in_port(p_midi_in, clk_midi);
 
  start_clock(clk_midi);
  start_port(p_midi_out);
  start_port(p_midi_in);
 
  reset_midi_state(mips);

  t :> time;
  t2 :> rxT;

#ifndef MIDI_LOOPBACK
  p_midi_out <: 1; // Start with high bit. 
  //  printstr("mout0");
#endif
 
  while (1) {
    int is_ack;
    unsigned int datum;
    select 
      {
        // Input to read the start bit
#ifndef MIDI_LOOPBACK
#ifdef MIDI_IN_4BIT_PORT
      case !isRX => p_midi_in when pinseq(0xE) :> void @  rxPT:
#else
      case !isRX => p_midi_in when pinseq(0) :> void @  rxPT:
#endif
        isRX = 1;
        t2 :> rxT;
        rxT += (bit_time + bit_time_2); 
        rxPT += (bit_time + bit_time_2); // absorb start bit and set to halfway through the next bit
        rxI = 0;
        asm("setc res[%0],1"::"r"(p_midi_in));
        asm("setpt res[%0],%1"::"r"(p_midi_in),"r"(rxPT));
        break;        
        // Input to read the remaining bits
      case isRX => t2 when timerafter(rxT) :> int _ :
        if (rxI++ < 8) 
          {
            // shift in bits into the high end of a word
            unsigned bit;
            p_midi_in :> bit;
            rxByte = (bit << 31) | (rxByte >> 1);
            rxT += bit_time;
            rxPT += bit_time;
            asm("setpt res[%0],%1"::"r"(p_midi_in),"r"(rxPT));            
          } 
        else 
          { 
            unsigned bit;
            // rcv and check stop bit
            p_midi_in :> bit;
            if ((bit & 0x1) == 1)
              {
                unsigned valid = 0;
                unsigned event = 0;
                uin_count++;
                rxByte >>= 24;
                //                if (rxByte != outputted_symbol) {
                //                  printhexln(rxByte);
                //                  printhexln(outputted_symbol);
                //                }

                {valid, event} = midi_in_parse(mips, cable_number, rxByte);
                if (valid && !got_next_event) {
                  event = byterev(event);   
                  // data to send to host - add to fifo
                  if (!waiting_for_ack) {
                    // send data         
                    //                    printstr("uart->decouple: ");  
                    outuint(c_midi, event);
                    waiting_for_ack = 1;
                    th_count++;
                  }
                  else {
                    next_event = event;
                    got_next_event = 1;
                  }
                }
                else if (valid) {
                  //                  printstr("g");
                }

              }
            isRX = 0;
          }
        break;
        
        // Output
        // If outputting then feed the bits out one at a time
        //  until symbol is zero expect pattern like 10'b1dddddddd0
        // This code will leave the output high afterwards due to the stop bit added with makeSymbol
      case outputting => t when timerafter(time) :> int _:
        if (symbol == 0) 
          {
            uout_count++;
            outputted_symbol = outputting_symbol;
            // have we got another symbol to send to uart?
            if (!isempty(q)) { // FIFO not empty
              // Take from FIFO
              outputting_symbol = dequeue(q);
              symbol = makeSymbol(outputting_symbol);

              if (space(q) > 3 && midi_from_host_overflow) {
                midi_from_host_overflow = 0;
                midi_send_ack(c_midi);
              }

              p_midi_out <: 1 @ txPT;
              //              printstr("mout1\n");
              t :> time;
              time += bit_time;
              txPT += bit_time;
            }
            else 
              outputting = 0;
          } 
        else 
          {
            time += bit_time;
            txPT += bit_time;
            p_midi_out @ txPT <: (symbol & 1);
            //            printstr("mout2\n");
            symbol >>= 1;
          }
        break;        
#endif

      case midi_get_ack_or_data(c_midi, is_ack, datum):
        if (is_ack) {
          // have we got more data to send
          //printstr("ack\n");
          if (got_next_event) {
            //printstr("uart->decouple\n");
            outuint(c_midi, next_event);
            th_count++;
            got_next_event = 0;
          }
          else {            
            waiting_for_ack = 0;
          }         
        }
        else {
          int event;
          unsigned midi[3];
          unsigned size;
          // received data from host
          event = byterev(datum);
          mr_count++;
#ifdef MIDI_LOOPBACK
  if (!got_next_event) {
    // data to send to host
    if (!waiting_for_ack) {
      // send data         
      event = byterev(event);                   
      outuint(c_midi, event);
      th_count++;
      waiting_for_ack = 1;
    }
    else {
      event = byterev(event);
      next_event = event;
      got_next_event = 1;
    }
  }
#else
          {midi[0], midi[1], midi[2], size} = midi_out_parse(event);
          for (int i = 0; i != size; i++) {
            // add symbol to fifo
            enqueue(q, midi[i]);
          }
 
          if (space(q) > 3) {
            midi_send_ack(c_midi);
          } else {
            midi_from_host_overflow = 1;
          }
 
          // Start sending from FIFO
          if (!isempty(q) && !outputting) {
            outputting_symbol = dequeue(q);
            symbol = makeSymbol(outputting_symbol);

            if (space(q) > 2 && midi_from_host_overflow) {
              midi_from_host_overflow = 0;
              midi_send_ack(c_midi);
            } 

#ifdef MIDI_LOOPBACK         
            handle_byte_from_uart(c_midi, mips, cable_number, got_next_event, next_event, waiting_for_ack, symbol);
#else   
            // Start sending byte (to be continued by outputting case)
            p_midi_out <: 1 @ txPT;
            t :> time;
            time += bit_time;
            txPT += bit_time;
            outputting = 1;
#endif
          }
#endif
        }
        break;
      }
  }
}


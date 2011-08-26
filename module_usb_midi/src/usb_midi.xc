#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include "usb_midi.h"
#include "midiinparse.h"
#include "midioutparse.h"
#include "queue.h"
#include "port32A.h"
#include "iAP.h"

//#define MIDI_LOOPBACK 1

static unsigned makeSymbol(unsigned data) {
    // Start and stop bits to the data packet
    //  like 10'b1dddddddd0
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

extern int ith_count; // Count of things sent to host

//// state for iAP
extern char iap_buffer[513]; // This should be just enough to hold a maximum size RetDevAuthenticationInfo (plus one for checksum when using transaction id as well) but this exceeds wMaxPacketSize
extern int iap_bufferlen;
extern int data_to_send;
//
//// state for GetDevAuthenticationInfo (global because returned on several RetDevAuthenticationInfos)
//char major = 0xf;
//char minor = 0xb;
//char cert[1920];
//unsigned short certLen = 0;
//int currentSectionIndex;
//int maxSectionIndex;
//
//// state for GetDevAuthenticationSignature
//int startup = 1;
extern unsigned authenticating;

extern port p_i2c_scl;
extern port p_i2c_sda;
#define p_midi_out p_i2c_scl
#define p_midi_in p_i2c_sda

void usb_midi(in port ?p_midi_inj, out port ?p_midi_outj,
              clock ?clk_midi,
              chanend c_midi,
              unsigned cable_number,
chanend c_iap, chanend ?c_i2c // iOS stuff
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
  unsigned midi_to_host_fifo_arr[1];

  unsigned outputting_symbol, outputted_symbol;

  struct midi_in_parse_state mips;

  // the symbol fifo (to go out of uart)
  queue symbol_fifo;
  unsigned symbol_fifo_arr[USB_MIDI_DEVICE_OUT_FIFO_SIZE];

  unsigned rxPT, txPT;
  int midi_from_host_overflow = 0;

  // iAP declarations

   // Buffers
   queue to_host_fifo;
   unsigned to_host_arr[256];
   queue from_host_fifo;
   unsigned from_host_arr[256];
   // State
   unsigned expecting_length = 1; // Expecting the next data item to be a length 
   unsigned expected_data_length = 0; // The length of data we are expecting



  //configure_clock_rate(clk_midi, 100, 1);
  init_queue(symbol_fifo, symbol_fifo_arr, USB_MIDI_DEVICE_OUT_FIFO_SIZE);
  init_queue(midi_to_host_fifo, midi_to_host_fifo_arr, 1);

  configure_out_port_no_ready(p_midi_out, clk_midi, 1);
  configure_in_port(p_midi_in, clk_midi);

  start_clock(clk_midi);
  start_port(p_midi_out);
  start_port(p_midi_in);

  reset_midi_state(mips);

  t :> txT;
  t2 :> rxT;

   //port32A_unset(P32A_I2C_NOTMIDI);
#ifndef MIDI_LOOPBACK
  p_midi_out <: 1; // Start with high bit.
  //  printstr("mout0");
#endif


   // iAP initialisation
   // Initialise buffers
   init_queue(to_host_fifo, to_host_arr, 256);
   init_queue(from_host_fifo, from_host_arr, 256);


  while (1) {
    int is_ack;
    int is_reset;
    unsigned int datum;
    select {
      // Input to read the start bit
#ifndef MIDI_LOOPBACK
#ifdef MIDI_IN_4BIT_PORT
      case (!authenticating && !isRX) => p_midi_in when pinseq(0xE) :> void @  rxPT:
#else
      case (!authenticating && !isRX) => p_midi_in when pinseq(0) :> void @  rxPT:
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
      case (!authenticating && isRX) => t2 when timerafter(rxT) :> int _ :
      {
        unsigned bit;
        p_midi_in :> bit;
        if (rxI++ < 8) {
            // shift in bits into the high end of a word
            rxByte = (bit << 31) | (rxByte >> 1);
            rxT += bit_time;
            rxPT += bit_time;
            asm("setpt res[%0],%1"::"r"(p_midi_in),"r"(rxPT));
        } else {
            // rcv and check stop bit
            if ((bit & 0x1) == 1) {
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
                if (valid && isempty(midi_to_host_fifo)) {
                  event = byterev(event);
                  // data to send to host - add to fifo
                  if (!waiting_for_ack) {
                    // send data
                    //                    printstr("uart->decouple: ");
                    outuint(c_midi, event);
                    waiting_for_ack = 1;
                    th_count++;
                  } else {
                    enqueue(midi_to_host_fifo, event);
                  }
                } else if (valid) {
                  //                  printstr("g");
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
        if (symbol == 0) {
            // Got something to output but not mid-symbol.
            // Start sending symbol.
            //  This case is reached when a symbol has been received from the host but not started AND
            //  When it has just finished sending a symbol

            // Take from FIFO
            outputting_symbol = dequeue(symbol_fifo);
            symbol = makeSymbol(outputting_symbol);

            if (space(symbol_fifo) > 3 && midi_from_host_overflow) {
              midi_from_host_overflow = 0;
              midi_send_ack(c_midi);
            }

            p_midi_out <: 1 @ txPT;
            //              printstr("mout1\n");
            t :> txT;
            txT += bit_time;
            txPT += bit_time;
            isTX = 1;
        } else {
            // Mid-symbol
            txT += bit_time; // Should this be after the output otherwise be double the length of the high before the start bit
            txPT += bit_time;
            p_midi_out @ txPT <: (symbol & 1);
            //            printstr("mout2\n");
            symbol >>= 1;
            if (symbol == 0) {
               // Finished sending byte
               uout_count++;
               outputted_symbol = outputting_symbol;
               if (isempty(symbol_fifo)) { // FIFO empty
                  isTX = 0;
               }
            }
        }
        break;
#endif

      case !authenticating => midi_get_ack_or_data(c_midi, is_ack, datum):
        if (is_ack) {
          // have we got more data to send
          //printstr("ack\n");
          if (!isempty(midi_to_host_fifo)) {
            //printstr("uart->decouple\n");
            outuint(c_midi, dequeue(midi_to_host_fifo));
            th_count++;
          } else {
            waiting_for_ack = 0;
          }
        } else {
          unsigned midi[3];
          unsigned size;
          // received data from host
          int event = byterev(datum);
          mr_count++;
#ifdef MIDI_LOOPBACK
  if (isempty(midi_to_host_fifo)) {
    // data to send to host
    if (!waiting_for_ack) {
      // send data
      event = byterev(event);
      outuint(c_midi, event);
      th_count++;
      waiting_for_ack = 1;
    } else {
      event = byterev(event);
      enqueue(midi_to_host_fifo, event);
    }
  }
#else
          {midi[0], midi[1], midi[2], size} = midi_out_parse(event);
          for (int i = 0; i != size; i++) {
            // add symbol to fifo
            enqueue(symbol_fifo, midi[i]);
          }

          if (space(symbol_fifo) > 3) {
            midi_send_ack(c_midi);
          } else {
            midi_from_host_overflow = 1;
          }
          // Drop through to the isTX guarded case
          if (!isTX) {
            t :> txT; // Should be enough to trigger the other case
            isTX = 1;
          }
#endif
        }
        break;
      case !(isTX || isRX) => iap_get_ack_or_reset_or_data(c_iap, is_ack, is_reset, datum):
         if (is_reset) {
authenticating = 1;
   // Start buffer with StartIDPS message in
   iap_bufferlen = StartIDPS(iap_buffer);
   port32A_set(P32A_I2C_NOTMIDI);
   for (int i = 0; i != iap_bufferlen; i++) {
      enqueue(to_host_fifo, iap_buffer[i]);
   }
   //dump(to_host_fifo);
   // Start the ball rolling (so I will be expecting an ack)
   outuint(c_iap, dequeue(to_host_fifo));
   ith_count++;

         } else {
         //printstrln("iap_get_ack_or_data");
         if (is_ack) {
           // have we got more data to send
           //printstr("ack\n");
           if (!isempty(to_host_fifo)) {
             //printstr("iap->decouple\n");
             outuint(c_iap, dequeue(to_host_fifo));
             ith_count++;
           } else {
             //printintln(ith_count);
           }
         } else {
            if (expecting_length) {
              expected_data_length = datum;
              expecting_length = 0;
              // iap_send_ack(c_iap); // Don't send ack as I don't expect it at the other end!
            } else {
              // Expecting data
              int fullness;
              enqueue(from_host_fifo, datum);
              iap_send_ack(c_iap);
              fullness = items(from_host_fifo);
              if (fullness == expected_data_length) {
                // Received whole message. Transfer to iap_buffer for parse
                for (int i = 0; i != fullness; i++) {
                   iap_buffer[i] = dequeue(from_host_fifo);
                }
                iap_bufferlen = expected_data_length;
                // Parse iAP from host
                parseiAP(c_i2c);
                // Start the ball rolling
                if (data_to_send) {
                   for (int i = 0; i != iap_bufferlen; i++) {
                      enqueue(to_host_fifo, iap_buffer[i]);
                      //printhexln(dequeue(from_host_fifo));
                   }
                   outuint(c_iap, dequeue(to_host_fifo));
                   ith_count++;
                   data_to_send = 0; // In this usage data_to_send is just used to kick off the data. The rest is pulled by ACKs.
                }
                expecting_length = 1;
              }
            }
         }
         if (!authenticating) {
 //           printstrln("Completed authentication");
              p_midi_in :> void; // Change port around to input again after authenticating
         }
         }
   
         break;
       }
  }
}


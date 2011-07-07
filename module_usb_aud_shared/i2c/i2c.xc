#include <xs1.h>
#include <print.h>

int I2cRegRead(int device, int addr, port scl, port sda)
{
    //int Result;
   timer gt;
   unsigned time;
   int Temp, CtlAdrsData, i;
   // three device ACK
   int ack[3];
   int rdData;

   // initial values.
   scl <: 1;
   sda  <: 1;
   sync(sda);
   gt :> time;
   time += 1000 + 1000;
   gt when timerafter(time) :> int _;
   // start bit on SDI
   scl <: 1;
   sda  <: 0;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> int _;
   scl <: 0;
   // shift 7bits of address and 1bit R/W (fixed to write).
   // WARNING: Assume MSB first.
   for (i = 0; i < 8; i += 1)
   {
      Temp = (device >> (7 - i)) & 0x1;
      sda <: Temp;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> int _;
      scl <: 1;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> int _;
      scl <: 0;
   }
   // turn the data to input
   sda :> Temp;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> int _;
   scl <: 1;
   // sample first ACK.
   sda :> ack[0];
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> int _;
   scl <: 0;
   
   CtlAdrsData = (addr & 0xFF);
   // shift first 8 bits.
   for (i = 0; i < 8; i += 1)
   {
      Temp = (CtlAdrsData >> (7 - i)) & 0x1;
      sda <: Temp;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> int _;
      scl <: 1;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> int _;
      scl <: 0;
   }
   // turn the data to input
   sda :> Temp;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> int _;
   scl <: 1;
   // sample second ACK.
   sda :> ack[1];
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> int _;
   scl <: 0;


   // stop bit
   gt :> time;
   time += 1000 + 1000;
   gt when timerafter(time) :> int _;
   // start bit on SDI
   scl <: 1;
   sda  <: 1;
   time += 1000 + 1000;
   gt when timerafter(time) :> int _;
   scl <: 0;
   time += 1000 + 1000;
   gt when timerafter(time) :> int _;
   
   
   // send address and read
   scl <: 1;
   sda  <: 1;
   sync(sda);
   gt :> time;
   time += 1000 + 1000;
   gt when timerafter(time) :> int _;
   // start bit on SDI
   scl <: 1;
   sda  <: 0;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> int _;
   scl <: 0;
   // shift 7bits of address and 1bit R/W (fixed to write).
   // WARNING: Assume MSB first.
   for (i = 0; i < 8; i += 1)
   {
      int deviceAddr = device | 1;
      Temp = (deviceAddr >> (7 - i)) & 0x1;
      sda <: Temp;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> int _;
      scl <: 1;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> int _;
      scl <: 0;
   }
   // turn the data to input
   sda :> Temp;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> int _;
   scl <: 1;
   // sample first ACK.
   sda :> ack[0];
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> int _;
   scl <: 0;
   
   
   rdData = 0;
   // shift second 8 bits.
   for (i = 0; i < 8; i += 1)
   {
      
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> int _;
      scl <: 1;

      sda :> Temp;
      rdData = (rdData << 1) | (Temp & 1);
      
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> int _;
      scl <: 0;
   }
   
   // turn the data to input
   sda :> Temp;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> int _;
   scl <: 1;
   // sample second ACK.
   sda :> ack[2];
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> int _;
   scl <: 0;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> int _;
   scl <: 1;
   // put the data to a good value for next round.
   sda  <: 1;
   // validate all items are ACK properly.
   //Result = 0;
   //for (i = 0; i < 3; i += 1)
   //{
      //if ((ack[i]&1) != 0)
      //{
         //Result = Result | (1 << i);
      //}
   //}

   return rdData;
}

void I2cRegWrite(int device, int addr, int data,  port scl, port sda)
{
   //int Result;
   timer gt;
   unsigned time;
   int Temp, CtlAdrsData, i;
   // three device ACK
   int ack[3];

   // initial values.
   scl <: 1;
   sda  <: 1;
   sync(sda);

   gt :> time;
   time += 1000 + 1000;
   gt when timerafter(time) :> void;

   // start bit on SDI
   scl <: 1;
   sda  <: 0;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> void;
   scl <: 0;

   // shift 7bits of address and 1bit R/W (fixed to write).
   // WARNING: Assume MSB first.
   for (i = 0; i < 8; i += 1)
   {
      Temp = (device >> (7 - i)) & 0x1;
      sda <: Temp;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> void;
      scl <: 1;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> void;
      scl <: 0;
   }

   // turn the data to input
   sda :> Temp;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> void;
   scl <: 1;

   // sample first ACK.
   sda :> ack[0];
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> void;
   scl <: 0;

   CtlAdrsData = (addr & 0xFF);

   // shift first 8 bits.
   for (i = 0; i < 8; i += 1)
   {
      Temp = (CtlAdrsData >> (7 - i)) & 0x1;
      sda <: Temp;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> void;
      scl <: 1;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> void;
      scl <: 0;
   }
   // turn the data to input
   sda :> Temp;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> void;
   scl <: 1;
   // sample second ACK.
   sda :> ack[1];
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> void;
   scl <: 0;

   CtlAdrsData = (data & 0xFF);
   // shift second 8 bits.
   for (i = 0; i < 8; i += 1)
   {
      Temp = (CtlAdrsData >> (7 - i)) & 0x1;
      sda <: Temp;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> void;
      scl <: 1;
      gt :> time;
      time += 1000;
      gt when timerafter(time) :> void;
      scl <: 0;
   }
   // turn the data to input
   sda :> Temp;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> void;
   scl <: 1;
   // sample second ACK.
   sda :> ack[2];
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> void;
   scl <: 0;
   gt :> time;
   time += 1000;
   gt when timerafter(time) :> void;
   scl <: 1;
   // put the data to a good value for next round.
   sda  <: 1;
   // validate all items are ACK properly.
   //Result = 0;
   //for (i = 0; i < 3; i += 1)
   //{
      //if ((ack[i]&1) != 0)
      //{
         //Result = Result | (1 << i);
      //}
   //}
   //return(Result);
}

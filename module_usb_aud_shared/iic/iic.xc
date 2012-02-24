/*
 * @ModuleName  iic
 * @Author  Ali Dixon, Corin Rathbone & Ross Owen
 * @Date  30/07/2008
 * @Version   1.2
 * @Description IIC bus interface
 *
 * The copyrights, all other intellectual and industrial
 * property rights are retained by XMOS and/or its licensors.
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2008
 */


#include <xs1.h>
#include <stdio.h>
#include "print.h"
#include "iic.h"
#include "freqConst.h"

#define EXPECT_ACK 0

#define IIC_WE 0
#define IIC_RE 1

#define iic_bittime                 (2000) // ref clock cycles per iic bit

// times are in units of 10ns
#define iic_scl_high_time           (iic_bittime >> 1)
#define iic_scl_low_time            (iic_bittime >> 1)
#define iic_bus_free_time           iic_bittime
#define iic_start_cond_setup_time   iic_bittime
#define iic_start_cond_hold_time    iic_bittime
#define iic_write_cycle_time        (5000 * XS1_TIMER_MHZ) //  5ms
#define iic_write_ack_poll_time     (  50 * XS1_TIMER_MHZ) // 50us

//extern port p_i2c_scl;
#define PORT_IIC_SCL p_scl
//extern port p_i2c_sda;
#define PORT_IIC_SDA p_sda

extern timer i2ctimer;

#define SCLHIGH() \
  { \
  int onedetect = 0; \
  while (1) { \
    int val; \
    PORT_IIC_SCL :> val; \
    if (val == 1) { \
      onedetect++; \
      if (onedetect > 20) \
        break; \
    } else { \
      onedetect = 0; \
    } \
  } \
  }

// Wait for the specified amount of time
void iic_phy_wait(unsigned delay, timer t)
{
  unsigned int time;
  t :> time;
  t when timerafter (time + delay) :> unsigned int tmp;
}


// Release the bus
int iic_initialise(timer t, port p_scl, port p_sda)
{
  int returnCode = 0;
  unsigned int scl_value;
  unsigned int sda_value;

  scl_value = 0;
  sda_value = 0;

  while ((scl_value != 1) || (sda_value != 1))
  {
    PORT_IIC_SCL :> scl_value;
    PORT_IIC_SDA :> sda_value;
  }

  iic_phy_wait(10 * iic_bittime, t);

  scl_value = 0;
  sda_value = 0;
  while ((scl_value != 1) || (sda_value != 1))
  {
    PORT_IIC_SCL :> scl_value;
    PORT_IIC_SDA :> sda_value;
  }

  return returnCode;
}


// Generate start condition
void iic_phy_master_start(timer t, port p_scl, port p_sda)
{
  SCLHIGH();
  PORT_IIC_SDA <: 1;
  iic_phy_wait(iic_start_cond_setup_time, t);

  PORT_IIC_SDA <: 0;
  iic_phy_wait(iic_start_cond_hold_time, t);

  PORT_IIC_SCL <: 0;
}


// Generate stop condition
void iic_phy_master_stop(timer t, port p_scl, port p_sda)
{
  PORT_IIC_SDA <: 0;
  iic_phy_wait(iic_scl_low_time, t);
  SCLHIGH();
  iic_phy_wait(iic_scl_high_time, t);

  PORT_IIC_SDA <: 1;
  iic_phy_wait(iic_bus_free_time, t);
}


// Send a bit
unsigned int iic_phy_sendBit(unsigned int bit, timer t, port p_scl, port p_sda)
{
  unsigned int time;

  t :> time;
  PORT_IIC_SDA <: bit;

  time += iic_scl_low_time;
  t when timerafter(time) :> time;
  SCLHIGH();
  t :> time;

  time += iic_scl_high_time;
  t when timerafter(time) :> time;
  PORT_IIC_SCL <: 0;        // set clock low

  return 0;
}


// Receive a bit
unsigned int iic_phy_receiveBit(timer t, port p_scl, port p_sda)
{
  unsigned int bit;
  unsigned int time;

  t :> time;
  PORT_IIC_SDA :> int tmp;

  time += iic_scl_low_time;
  t when timerafter(time) :> time;
  SCLHIGH();
  t :> time;

  PORT_IIC_SDA :> bit;

  time += iic_scl_high_time;
  t when timerafter(time) :> time;
  PORT_IIC_SCL <: 0;        // set clock low

  time += iic_scl_low_time;
  t when timerafter(time) :> time;
  return bit;
}


// Send a byte
int iic_phy_sendByte(unsigned int inByte, unsigned int expectAck, timer t, port p_scl, port p_sda)
{
  unsigned int bitCount;
  unsigned int bit;
  unsigned int byte;
  unsigned int ack;
  unsigned int time;

  byte = inByte;
  bitCount = 0;

  t :> time;

  while (bitCount < 8)
  {

    bit = (byte & 0x80) >> 7;
    time += (iic_scl_low_time>>1);
    t when timerafter(time) :> time;

    PORT_IIC_SDA <: bit;

    time += (iic_scl_low_time>>1);
    t when timerafter(time) :> time;

    SCLHIGH();
    t :> time;

    byte = byte << 1;
    bitCount = bitCount + 1;

    time += iic_scl_high_time;
    t when timerafter(time) :> time;
    PORT_IIC_SCL <: 0;        // set clock low
  }

  PORT_IIC_SDA :> int tmp;

  ack = iic_phy_receiveBit(t, p_scl, p_sda);

  if (ack != expectAck)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


// Receive a byte
unsigned int iic_phy_receiveByte(unsigned int ack, timer t, port p_scl, port p_sda)
{
  unsigned int bitCount;
  unsigned int bit;
  unsigned int time;
  unsigned int byte;

  byte = 0;
  bitCount = 0;

  t :> time;

  // set to input
  PORT_IIC_SDA :> int tmp;

  while (bitCount < 8)
  {
    time += iic_scl_low_time;
    t when timerafter(time) :> time;
    SCLHIGH();
    t :> time;

    PORT_IIC_SDA :> bit;

    byte = (byte << 1) | bit;
    bitCount = bitCount + 1;

    time += iic_scl_high_time;
    t when timerafter(time) :> time;
    PORT_IIC_SCL <: 0;        // set clock low
  }

  PORT_IIC_SDA :> int tmp;

  iic_phy_sendBit(ack, t, p_scl, p_sda);

  return byte;
}


// Poll to determine when write completes
unsigned int iic_checkWriteComplete(unsigned int address, timer t, port p_scl, port p_sda)
{
    unsigned int time;

    t :> time;

    iic_phy_master_start(t, p_scl, p_sda);
    do
    {
        iic_phy_master_stop(t, p_scl, p_sda);
        t when timerafter(time + iic_write_ack_poll_time) :> time;
        iic_phy_master_start(t, p_scl, p_sda);
    }  while (iic_phy_sendByte((address << 1) | IIC_WE, EXPECT_ACK, t, p_scl, p_sda));

    if (iic_phy_sendByte(0, EXPECT_ACK, t, p_scl, p_sda))
    {
    }

    if (iic_phy_sendByte(0, EXPECT_ACK, t, p_scl, p_sda))
    {
    }

    iic_phy_master_stop(t, p_scl, p_sda);
    iic_phy_master_start(t, p_scl, p_sda);

    iic_phy_sendByte((address << 1) | IIC_WE, EXPECT_ACK, t, p_scl, p_sda);
    return 0;
}


// Write to IIC device
int iic_writeC(unsigned int address, unsigned int reg, char data[], unsigned int numBytes, chanend ?c,
    port ?p_scl, port ?p_sda)
{

    /* If null channel end arg passed in use i2c ports */
    if(isnull(c))
    {
        return iic_write(address, reg, data, numBytes, i2ctimer, p_scl, p_sda);
    }
    else
    {
        int read;
        int retVal;
        c <: 1; // isWrite
        c <: (address << 1);
        c <: reg;
        c <: numBytes;
        for(int i = 0; i!= numBytes; i++) 
        {
            read = data[i];
            c <: read;
        }

        c :> retVal;
        return retVal;
    }
}

void iic_wait(timer t, unsigned us)
{
	unsigned time;
	t :> time;
	time += us * 100;
	t when timerafter(time) :> void;
}
#define COPROCESSOR_LOOPS 10
int iic_write(unsigned int address, unsigned int reg, char data[], unsigned int numBytes, timer t, port p_scl, port p_sda)
{
    unsigned int i = 0;
    //printf("Start of iic_write(0x%04x, 0x%04x, 0x%02x, %d)\n", address, reg, data[0], numBytes);
    iic_phy_master_start(t, p_scl, p_sda);

    i = COPROCESSOR_LOOPS;
    while(i) 
    {
	    iic_phy_master_start(t, p_scl, p_sda);
	    if(0 == iic_phy_sendByte((address << 1) | IIC_WE, EXPECT_ACK, t, p_scl, p_sda))
		    break;
	    iic_wait(t, 600);
	    i--;
    }
    if(!i) 
    {
	    //printstrln("W1");
	    return 1;
    }

  if (iic_phy_sendByte((reg), EXPECT_ACK, t, p_scl, p_sda))
    return 3;

  for ( i=0; i<numBytes; i++)
  {
    if (iic_phy_sendByte(data[i], EXPECT_ACK, t, p_scl, p_sda))
      break;
  }

  iic_phy_master_stop(t, p_scl, p_sda);
  iic_checkWriteComplete(address, t, p_scl, p_sda);

  return 0;
}

// Read from IIC device
// It receives numBytes, acking each one and does not ack the final byte
int iic_readC(unsigned int address, unsigned int reg, char data[], unsigned int numBytes, chanend ?c,
    port ?p_scl, port ?p_sda)
{
    /* If null channend argument passed in use i2c ports */
    if(isnull(c))
    {
        return iic_read(address, reg, data, numBytes, i2ctimer, p_scl, p_sda);
    }
    else
    {
        int read;
        int retVal;
        c <: 0; // isWrite
        c <: (address << 1); // This is getting silly, this shifts it up to a full 8 bit address so i2c_thread can shift it down again so the real iic_read can shift it up again.
        c <: reg;
        c <: numBytes;
        for(int i = 0; i!= numBytes; i++) 
        {
            c :> read;
            data[i] = read;
        }
        c :> retVal;
        return retVal;
    }
}

int iic_read(unsigned int address, unsigned int reg, char data[], unsigned int numBytes, timer t, port p_scl, port p_sda)
{
    unsigned int i;

    iic_phy_master_start(t, p_scl, p_sda);

    i = COPROCESSOR_LOOPS;
    while(i) 
    {
	    iic_phy_master_start(t, p_scl, p_sda);
	    if(0 == iic_phy_sendByte((address << 1) | IIC_WE, EXPECT_ACK, t, p_scl, p_sda))
		    break;
	    iic_wait(t, 600);
	    i--;
    }
    if(!i) 
    {
	    //printstrln("S1");
	    return 1;
    }

    if (iic_phy_sendByte((reg), EXPECT_ACK, t, p_scl, p_sda)) 
    {
	    //printstrln("S2");
        return 1;
    }
    iic_phy_master_stop(t, p_scl, p_sda);

    i = COPROCESSOR_LOOPS;
    while(i) 
    {
	    iic_phy_master_start(t, p_scl, p_sda);
	    if(0 == iic_phy_sendByte((address << 1) | IIC_RE, EXPECT_ACK, t, p_scl, p_sda))
		    break;
	    iic_wait(t, 600);
	    i--;
    }
    if(!i) 
    {
	    //printstrln("S3");
	    return 1;
    }

    for (i=0; i<numBytes-1; i++)
    {
        data[i] = iic_phy_receiveByte(0, t, p_scl, p_sda);
    }
    // printf("Start of iic_read(0x%04x, 0x%04x, 0x%02x, %d)\n", address, reg, data[0], numBytes);

    // receive final byte and dont ack to signal end of transfer
    data[i] = iic_phy_receiveByte(1, t, p_scl, p_sda);
    iic_phy_master_stop(t, p_scl, p_sda);

    return 0;
}

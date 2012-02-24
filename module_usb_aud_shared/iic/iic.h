#ifndef IIC_H_
#define IIC_H_

#include <xccompat.h>

int iic_initialise(timer t, port p_scl, port p_sda);

// Write to IIC device
#if defined __XC__
int iic_writeC(unsigned int address, unsigned int reg, unsigned char data[], unsigned int numBytes, chanend ?c,
        port ?p_scl, port ?p_sda);
#elif defined __STDC__
int iic_writeC(unsigned int address, unsigned int reg, unsigned char data[], unsigned int numBytes, chanend c, 
        port p_scl, port p_sda);
#endif

int iic_write(unsigned int address, unsigned int reg, unsigned char data[], unsigned int numBytes, timer t,
        port p_scl, port p_sda);

// Read from IIC device
// It receives numBytes, acking each one and does not ack the final byte
#if defined __XC__
int iic_readC(unsigned int address, unsigned int reg, unsigned char data[], unsigned int numBytes, chanend ?c,
        port ?p_scl, port ?p_sda);
#elif defined __STDC__
int iic_readC(unsigned int address, unsigned int reg, unsigned char data[], unsigned int numBytes, chanend c,
        port p_scl, port p_sda);
#endif

int iic_read(unsigned int address, unsigned int reg, unsigned char data[], unsigned int numBytes, timer t, port p_scl, port p_sda);

// iic_phy_wait
// Wait a certain number of cycles
void iic_phy_wait(unsigned delay, timer t);

#endif /*IIC_H_*/

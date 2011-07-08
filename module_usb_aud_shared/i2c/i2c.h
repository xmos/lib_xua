
// Channel interface
void I2cRegWriteC(int deviceAdrs, int Adrs, int WrData,  chanend c);

int I2cRegReadC(int deviceAdrs, int Adrs, chanend c);

// Function interface
void I2cRegWrite(int deviceAdrs, int Adrs, int WrData,  port AUD_SCLK, port AUD_SDIN);

int I2cRegRead(int deviceAdrs, int Adrs, port AUD_SCLK, port AUD_SDIN);

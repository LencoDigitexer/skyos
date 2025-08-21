#include "uart.h"

unsigned int ser_UARTType(unsigned int port)
{
  outportb( port + SER_LINE_CONTROL, 0xAA);
  if (inportb(port + SER_LINE_CONTROL) != 0xAA) return NOSER;

  return 1;
}

unsigned int ser_init(unsigned int port, unsigned int baud,
					  unsigned char param)
{
	unsigned short divisor;

	divisor = SER_MAXBAUD / baud;
	outportb(port + SER_LINE_CONTROL, inportb(SER_LINE_CONTROL) | SER_LCR_SETDIVISOR);

	outportb(port + SER_DIVISOR_LSB, divisor & 255);
        outportb(port + SER_DIVISOR_MSB, divisor >> 8);

	outportb(port + SER_LINE_CONTROL, inportb(SER_LINE_CONTROL) & ~SER_LCR_SETDIVISOR);

	outportb(port + SER_LINE_CONTROL, param);

	inportb(port + SER_TXBUFFER);
    return 1;
}


unsigned int ser_rts(unsigned int port)
{
  return (inportb(port + SER_LINE_STATUS) & SER_LSR_TSREMPTY);
}

unsigned int ser_send(unsigned int port, unsigned char byte,
                      unsigned int timeout, unsigned char sigmask,
                      unsigned char sigvals)
{
  if (timeout)
  {
    while (!ser_rts(port) && timeout) timeout--;
    if (!timeout) return 0;
  }
  else
    while(!ser_rts(port));

  if ((inportb(port + SER_MODEM_STATUS) & sigmask) == sigvals)
  {
    outportb(port + SER_TXBUFFER, byte);
    return inportb(port + SER_LINE_STATUS) & SER_LSR_ERRORMSK;
  }
  return 0;
}

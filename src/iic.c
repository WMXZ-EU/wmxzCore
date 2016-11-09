/*
 * WMXZ Teensy core library
 * Copyright (c) 2016 Walter Zimmer.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//iic.c

#include <stdint.h>
#include "kinetis.h"
#include "core_pins.h"
#include "iic.h"

#define BUFFER_LENGTH 32

static int m_i2c_isRunning=0;

void i2c_init(void)
{
	SIM_SCGC4 |= SIM_SCGC4_I2C0;
}

void IIC_setup(int alt) // inspired by TwoWire
{
	if (m_i2c_isRunning) return;
	//
	I2C0_C1 = 0;
	
	if(alt==0)
	{
		CORE_PIN18_CONFIG = PORT_PCR_MUX(2)|PORT_PCR_ODE|PORT_PCR_SRE|PORT_PCR_DSE;
		CORE_PIN19_CONFIG = PORT_PCR_MUX(2)|PORT_PCR_ODE|PORT_PCR_SRE|PORT_PCR_DSE;
	}
	else
	{
		CORE_PIN16_CONFIG = PORT_PCR_MUX(2)|PORT_PCR_ODE|PORT_PCR_SRE|PORT_PCR_DSE;
		CORE_PIN17_CONFIG = PORT_PCR_MUX(2)|PORT_PCR_ODE|PORT_PCR_SRE|PORT_PCR_DSE;
	}
#if F_BUS == 60000000
	I2C0_F = 0x2C;	// 104 kHz
	// I2C0_F = 0x1C; // 416 kHz
	// I2C0_F = 0x12; // 938 kHz
	I2C0_FLT = 4;
#elif F_BUS == 56000000
	I2C0_F = 0x2B;	// 109 kHz
	// I2C0_F = 0x1C; // 389 kHz
	// I2C0_F = 0x0E; // 1 MHz
	I2C0_FLT = 4;
#elif F_BUS == 54000000
	I2C0_F = 0x2B;	// 109 kHz
	// I2C0_F = 0x1C; // 389 kHz
	// I2C0_F = 0x0E; // 1 MHz
	I2C0_FLT = 4;
#elif F_BUS == 48000000
	// I2C0_F = 0x27;	// 100 kHz
	I2C0_F = 0x1A; // (112)  428 kHz
	// I2C0_F = 0x0D; // 1 MHz
	I2C0_FLT = 4;
#elif F_BUS == 40000000
	I2C0_F = 0x29;	// 104 kHz
	// I2C0_F = 0x19; // 416 kHz
	// I2C0_F = 0x0B; // 1 MHz
	I2C0_FLT = 3;
#elif F_BUS == 36000000
	//I2C0_F = 0x28;	// 113 kHz
	I2C0_F = 0x15; // (88) 409 kHz
	// I2C0_F = 0x0A; // 1 MHz
	I2C0_FLT = 3;
#elif F_BUS == 24000000
	I2C0_F = 0x1F; // 100 kHz
	// I2C0_F = 0x12; // 375 kHz
	// I2C0_F = 0x02; // 1 MHz
	I2C0_FLT = 2;
#elif F_BUS == 16000000
	I2C0_F = 0x20; // 100 kHz
	// I2C0_F = 0x07; // 400 kHz
	// I2C0_F = 0x00; // 800 MHz
	I2C0_FLT = 1;
#elif F_BUS == 8000000
	I2C0_F = 0x14; // 100 kHz
	// I2C0_F = 0x00; // 400 kHz
	I2C0_FLT = 1;
#elif F_BUS == 4000000
	I2C0_F = 0x07; // 100 kHz
	// I2C0_F = 0x00; // 200 kHz
	I2C0_FLT = 1;
#elif F_BUS == 2000000
	I2C0_F = 0x00; // 100 kHz
	I2C0_FLT = 1;
#else
#error "F_BUS must be 60, 56, 54, 48, 40, 36, 24, 16, 8, 4 or 2 MHz"
#endif
	I2C0_C2 = I2C_C2_HDRS;
	I2C0_C1 = I2C_C1_IICEN;

	m_i2c_isRunning=1;
}

void i2c_resetBus(int alt) // inspired from i2c_t3
{
    uint8_t scl=0, sda=0, count=0;
	
	if(alt==0)
	{   sda = 18;
        scl = 19;
	}
	else
	{   sda = 17;
        scl = 16;
	}
	
        // change pin mux to digital I/O
        pinMode(sda,INPUT);
        pinMode(scl,OUTPUT);
        digitalWrite(scl,HIGH);

        while(digitalRead(sda) == 0 && count++ < 10)
        {
            digitalWrite(scl,LOW);
            delayMicroseconds(5);       // 10us period == 100kHz
            digitalWrite(scl,HIGH);
            delayMicroseconds(5);
        }
	m_i2c_isRunning=0;
	IIC_setup(alt);
}


// Chapter 44: Inter-Integrated Circuit (I2C) - Page 1012
//  I2C0_A1      // I2C Address Register 1
//  I2C0_F       // I2C Frequency Divider register
//  I2C0_C1      // I2C Control Register 1
//  I2C0_S       // I2C Status register
//  I2C0_D       // I2C Data I/O register
//  I2C0_C2      // I2C Control Register 2
//  I2C0_FLT     // I2C Programmable Input Glitch Filter register

inline void IIC_becomeMaster(void)
{
	// clear the status flags
	I2C0_S = I2C_S_IICIF | I2C_S_ARBL;
	// now take control of the bus...
	if (I2C0_C1 & I2C_C1_MST) 
	{   // we are already the bus master, so send a repeated start
		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_RSTA | I2C_C1_TX;
	} 
	else 
	{   // we are not currently the bus master, so wait for bus ready
		while (I2C0_S & I2C_S_BUSY) ;
		// become the bus master in transmit mode (send start)
		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
	}
}

inline void IIC_enableInterrupt(void)
{
	// enable interrupt
	I2C0_C1 |= I2C_C1_IICIE;
	//
    NVIC_ENABLE_IRQ(IRQ_I2C0);
}
inline void i2c_wait(void){	while (!(I2C0_S & I2C_S_IICIF)); I2C0_S = I2C_S_IICIF;}

typedef struct 
{	uint8_t address;
	I2C_FUNC funct;
	uint8_t *data;
	int ndat;
	int sendStop;
	int count;
} I2C_STRUCT;
//
I2C_STRUCT I2C_TX, I2C_RX;

uint8_t m_i2c_xferMode=0;
uint8_t m_i2c_error=0;
uint8_t m_i2c_done=0;

uint8_t i2c_error(void) {return I2C0_S;}
uint8_t i2c_done(void) {return m_i2c_done;}
uint8_t i2c_txCount(void){ return I2C_TX.count;}
uint8_t i2c_rxCount(void){ return I2C_RX.count;}

void i2c_send(uint8_t address, uint8_t *data, int ndat, int sendStop, I2C_FUNC funct)
{	I2C_TX.address=address;
	I2C_TX.funct=funct;
	I2C_TX.data=data;
	I2C_TX.ndat=ndat;
	I2C_TX.sendStop=sendStop;
	I2C_TX.count=0;
	m_i2c_xferMode=1;
	m_i2c_error=0;
	m_i2c_done=0;
	//
	IIC_becomeMaster();
	//
	IIC_enableInterrupt();
	//
	// transmit address
	I2C0_D = (address<<1); // address
	return;
}


void i2c_receive(uint8_t address, uint8_t *data, int ndat, int sendStop, I2C_FUNC funct)
{	I2C_RX.address=address;
	I2C_RX.funct=funct;
	I2C_RX.data=data;
	I2C_RX.ndat=ndat;
	I2C_RX.sendStop=sendStop;
	I2C_RX.count=0;
	m_i2c_xferMode=2;
	m_i2c_error=0;
	m_i2c_done=0;
	//
	IIC_becomeMaster();
	//
	IIC_enableInterrupt();
	//
	// transmit address
	I2C0_D = ((address << 1) | 1);		// Write 7-bit Slave Address + READ bit
}

void i2c0_isr(void)
{ 	uint8_t status;
	//
//	i2c_wait(); //check and clear intr
	// check status
	status = I2C0_S;
	I2C0_S |= I2C_S_IICIF;	// Clear interrupt flag
	
//	if (status & I2C_S_RXAK) { I2C0_C1 = I2C_C1_IICEN;  m_i2c_error=1; return;}
	if (status & I2C_S_ARBL) { I2C0_C1 = I2C_C1_IICEN;  I2C0_S |= I2C_S_ARBL; m_i2c_error=2; return;}
	//
	if(m_i2c_xferMode==1)
	{	// transmitting
		
		if(I2C_TX.count<I2C_TX.ndat)
			I2C0_D = I2C_TX.data[I2C_TX.count++];
		// if requested send stop
		if (I2C_TX.count==I2C_TX.ndat) 
		{	m_i2c_done=1;
			// disable interrupt
			delayMicroseconds(1);
			if(I2C_TX.sendStop==1)	
				I2C0_C1 = I2C_C1_IICEN;// send STOP, change to Rx mode, intr disabled
			// we have finished; execute callback
			if(I2C_TX.funct)
				I2C_TX.funct(0,0);
			I2C_TX.funct=0;
		}
	}
	else if(m_i2c_xferMode==2)
	{	// receiving
		
		if(I2C0_C1 & I2C_C1_TX) // still in transmitter mode (address sent)?
		{
			if((status & I2C_S_RXAK) == 0) // ACK Received?
			{
				I2C0_C1 &= ~I2C_C1_TX;	// Change to receiver mode
				I2C0_D;	// Dummy read to start reception
			}
			else
			{
				m_i2c_error=3; // NACK received
                m_i2c_done=1;
				delayMicroseconds(1); // empirical patch, lets things settle before issuing STOP
				I2C0_C1 &= ~I2C_C1_MST;	// Generate STOP signal
			}
		}
		else
		{	
			if(I2C_RX.count == (I2C_RX.ndat-1))	// Last byte to be received?
			{
                m_i2c_done=1;
				delayMicroseconds(1); // empirical patch, lets things settle before issuing STOP
				if(I2C_RX.sendStop)
					I2C0_C1 &= ~I2C_C1_MST;	// Generate STOP signal
			}
			else
			{
				if(I2C_RX.count == (I2C_RX.ndat-2))	// Only 1 more byte pending to read?
				{
					I2C0_C1 |= I2C_C1_TXAK;	// Generate NACK in the next reception
				}
			}
			I2C_RX.data[I2C_RX.count++] = I2C0_D;	// Copy data register to buffer
			//
			// we have finished; execute callback
			if(I2C_RX.count==I2C_RX.ndat) 
			{	if(I2C_RX.funct)
					I2C_RX.funct(I2C_RX.data,I2C_RX.ndat);
				I2C_RX.funct=0;
			}
		}
	}
}

int IIC_write(const uint8_t *data, int ndat, uint8_t address, int sendStop)
{	int ii;
	uint8_t status, ret=0;
	//
	IIC_becomeMaster();
	// transmit address
	I2C0_D = (address<<1);
	i2c_wait();
	status = I2C0_S;
	if (status & I2C_S_RXAK) { ret=2; return ret;}
	
	// transmit data
	for (ii=0; ii < ndat; ii++) 
	{   I2C0_D = data[ii];
		i2c_wait();
		status = I2C0_S;
		if (status & I2C_S_RXAK) 
		{ 
			ret = 3; // 3:received NACK on transmit of data 
			break;
		}
		if ((status & I2C_S_ARBL)) 
		{
			// we lost bus arbitration to another master
			// TODO: what is the proper thing to do here??
			ret = 4; // 4:other error 
			break;
		}
	}
	delayMicroseconds(10);
	if (sendStop) { I2C0_C1 = I2C_C1_IICEN;	}
	
	return ret;
}

int IIC_read(uint8_t *data, int length, uint8_t address, int sendStop)
{ int count=0;
	uint8_t status;
	//
	IIC_becomeMaster();
	
	// send the address
	I2C0_D = (address << 1) | 1; //add read flag
	i2c_wait();
	status = I2C0_S;
	if ((status & I2C_S_RXAK) || (status & I2C_S_ARBL)) 
	{	// the slave device did not acknowledge
		// or we lost bus arbitration to another master
		I2C0_C1 = I2C_C1_IICEN;
		return 0;
	}
	//
	if (length == 1) 
	{	I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TXAK;
	} 
	else 
	{	I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST;
	}
	
	I2C0_D; // initiate the first receive
	while (length > 1) 
	{	i2c_wait();
		length--;
		if (length == 1) I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TXAK;
		data[count++] = I2C0_D;
	}
	i2c_wait();
	I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
	data[count++] = I2C0_D;
	//
	delayMicroseconds(10);
	if (sendStop) I2C0_C1 = I2C_C1_IICEN;
	//
	return count;
}

uint8_t writeRegister(uint8_t address, uint8_t reg, uint8_t value)
{ uint8_t data[2], err=0;
  data[0]=reg;  data[1]=value;
  err=IIC_write(data, 2,address,1);
  if(err>0)  return err;
  return 0;
}

uint8_t readRegister(uint8_t address, uint8_t reg)
{ uint8_t data[2], err=0;
  data[0]=reg;
  err=IIC_write(data, 1,address,0);
  if(err>0) return err;
  //
  IIC_read(data, 1, address,1);
  return data[0];
}

uint8_t i2cWriteRegister8(uint8_t address, uint8_t reg, uint8_t value)
{ uint8_t data[2], ret=0;
  data[0]=reg;  data[1]=value;
  ret=IIC_write(data, 2,address,1);
  return ret;
}

uint8_t i2cReadRegister8(uint8_t address, uint8_t reg, uint8_t *value)
{ uint8_t data[2], ret=0;
  data[0]=reg;
  ret=IIC_write(data, 1,address,0);
  if(ret>0) return 0;
  //
  ret=IIC_read(data, 1, address,1);
  *value=data[0];
  return ret;
}

uint8_t i2cReadRegister8s(uint8_t address, uint8_t reg, uint8_t *value)
{ uint8_t data[2], ret=0;
  data[0]=reg;
  ret=IIC_write(data, 1,address,1); // put stop after write
  if(ret>0) return 0;
  //
  ret=IIC_read(data, 1, address,1);
  *value=data[0];
  return ret;
}

uint8_t i2cWriteRegister16(uint8_t address, uint16_t reg, uint16_t value)
{ uint8_t data[4], ret=0;
  data[0]=reg>>8;  data[1]=reg; data[2]=value>>8; data[3]=value;
  ret=IIC_write(data, 4,address,1);
  return ret;
}

uint8_t i2cReadRegister16(uint8_t address, uint16_t reg, uint16_t *value)
{ uint8_t data[2], ret=0;
  data[0]=reg>>8; data[1]=reg;
  ret=IIC_write(data, 1,address,0);
  if(ret>0) return 0;
  //
  ret=IIC_read(data, 2, address,1);
  *value= data[0]<<8 | data[1];
  return ret;
}


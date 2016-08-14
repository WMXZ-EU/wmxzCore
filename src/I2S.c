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
//i2s.c
#include "kinetis.h"
#include "core_pins.h"

#include "i2s.h"

#include "dma.h"
#include "jobs.h"

#define HAVE_HW_SERIAL
#ifdef HAVE_HW_SERIAL

#include "kinetis.h"
#include "localKinetis.h"
#include "core_pins.h" // testing only

/* some aux functions for pure c code */
#include "usb_serial.h"

#define _PUTCHAR(x) usb_serial_putchar(x)
#define _FLUSH() usb_serial_flush_output()
//
void logg(char c);
void printb(uint32_t x,uint32_t im);
//
void logg(char c) {_PUTCHAR(c); _FLUSH();}
void printb(uint32_t x,uint32_t im)
{ char c;
  int ii;
  for(ii=im;ii>=0;ii--)
  { if(!((ii+1)%4)) _PUTCHAR(' ');
    c=(x&(1<<ii))?'1':'0'; _PUTCHAR(c);
  }
  _PUTCHAR('\r');
  _PUTCHAR('\n');
  _FLUSH();
}
/* end aux functions */
#endif


int m_i2s_isMaster;
int m_i2s_nbits;
int m_i2s_dual;

void i2s_init(void)
{	SIM_SCGC6 |= SIM_SCGC6_I2S;
}

void i2s_config(int device, int isMaster, int nbits, int fs_scale, int dual, int sync)
{
// rules to generate click dividers
//  MCGPLLCLK=F_CPU // is set by _MICS(3)
//  MCLK= MCGPLLCLK*(iscl1+1)/(iscl2+1)
//	BCLK= MCLK/2/(iscl3+1)/32; // division by 32 is to have 32 bits within frame sync (BCLK)
//
	int iscl1,iscl2,iscl3;
	if(device==CS5361_DEV)
	{
		// adjust speeds to requirements by cs5361
		int cs_speed=1;
		if(fs_scale>3)
			cs_speed=1;		// fs < 54 kHz
		else if(fs_scale>1)
			cs_speed=2;		// fs > 50 kHz & fs < 104 kHz
		else
			cs_speed=4;  	// fs >100 kHz % fs < 204 kHz
		
		// following timings are relative 200 kHz @144 MHz
		iscl1 = nbits/cs_speed-1;
		iscl2 = fs_scale*3*15-1;  
		iscl3 = 4/cs_speed-1;
		iscl1=3;
		iscl2=50;
		iscl3=3;	

	}
	else if(device==SGTL5000_DEV)
	{
		// following timings are relative 96 kHz @144 MHz
		iscl1 = 6-1;
		iscl2 = fs_scale*35;  
		iscl3 = 4-1;
	}
	else if(device==AD7982_DEV)
	{
		iscl1 = 6-1;
		iscl2 = fs_scale*35;  
		iscl3 = 4-1;
	}
	else
	{ // 44.1 kHz @ 144 MHz //CMIS fft (OK) OK with for processing
		iscl1=3;
		iscl2=50;
		iscl3=3;	
	}

	// if either transmitter or receiver is enabled, do nothing
	if (I2S0_TCSR & I2S_TCSR_TE) return;
	if (I2S0_RCSR & I2S_RCSR_RE) return;

	I2S0_TCSR=0;
	I2S0_RCSR=0;
	//
	m_i2s_isMaster=isMaster;
	m_i2s_nbits=nbits;
	m_i2s_dual=dual;
	
	// enable MCLK output
	if(isMaster)
	{
		I2S0_MCR = I2S_MCR_MICS(3)  | I2S_MCR_MOE;
		while (I2S0_MCR & I2S_MCR_DUF) ; 
		I2S0_MDR = I2S_MDR_FRACT(iscl1) | I2S_MDR_DIVIDE(iscl2); 
	}

	// configure transmitter
	I2S0_TMR = 0;
	I2S0_TCR1 = I2S_TCR1_TFW(1);  // watermark at half fifo size
	I2S0_TCR2 = I2S_TCR2_SYNC((1-sync)) | I2S_TCR2_BCP ; // sync=1; tx is async; rx = sync
	if(isMaster)
		I2S0_TCR2 |= (I2S_TCR2_BCD | I2S_TCR2_DIV(iscl3) | I2S_TCR2_MSEL(1));
	//
	if(dual&2)
		I2S0_TCR3 = I2S_TCR3_TCE_2CH; // dual tx channel
	else
		I2S0_TCR3 = I2S_TCR3_TCE; // single tx channel
	//
	I2S0_TCR4 = I2S_TCR4_FRSZ(1) 
				| I2S_TCR4_SYWD((nbits-1)) 
				| I2S_TCR4_MF 
				| I2S_TCR4_FSE 
				| I2S_TCR4_FSP ;
	if(isMaster)
		I2S0_TCR4 |= I2S_TCR4_FSD;	
	
	I2S0_TCR5 = I2S_TCR5_WNW((nbits-1)) | I2S_TCR5_W0W((nbits-1)) | I2S_TCR5_FBT((nbits-1));

	// configure receiver 
	I2S0_RMR = 0;
	I2S0_RCR1 = I2S_RCR1_RFW(1); 
	I2S0_RCR2 = I2S_RCR2_SYNC(sync) | I2S_RCR2_BCP ; // sync=0; rx is async; tx is sync
	if(isMaster)
		I2S0_RCR2 = (I2S_RCR2_BCD | I2S_RCR2_DIV(iscl3) | I2S_RCR2_MSEL(1));
	//
	if(dual&1)
		I2S0_RCR3 = I2S_RCR3_RCE_2CH; // dual rx channel
	else
		I2S0_RCR3 = I2S_RCR3_RCE; // single rx channel
	//
	I2S0_RCR4 = I2S_RCR4_FRSZ(1) 
				| I2S_RCR4_SYWD((nbits-1)) 
				| I2S_RCR4_MF
				| I2S_RCR4_FSE  
				| I2S_RCR4_FSP;
	if(isMaster)
		I2S0_RCR4 |= I2S_RCR4_FSD;	

	I2S0_RCR5 = I2S_RCR5_WNW((nbits-1)) | I2S_RCR5_W0W((nbits-1)) | I2S_RCR5_FBT((nbits-1));
}

// Pin patterns
//      pin     alt4            alt6
//      3                       I2S0_TXD0           (transmit data, also on 22)
//      4                       I2S0_TX_FS          (transmit word clock, also on 23, 25)
//      9                       I2S0_TX_BCLK        (transmit bit clock, also on 24, 32)
//      11      I2S0_RX_BCLK    I2S0_MCLK           (receive bit clock, also on 27; or master clock, also on 28)
//      12      I2S0_RX_FS                          (receive word clock, also on 29)
//      13      I2S0_RXD0                           (receive data)
//      22                      I2S0_TXD0           (transmit data, also on 3)
//      23                      I2S0_TX_FS          (also on 4, 25)
//      24                      I2S0_TX_BCLK        (also on 9, 32)
//      25                      I2S0_TX_FS          (also on 4, 23)
//      27      I2S0_RX_BCLK                        (also on 11)
//      28      I2S0_MCLK                           (also on 11)
//      29      I2S0_RX_FS                          (also on 12)
//      30      I2S0_RX_RDR1
//      32      I2S0_TX_BCLK                        (also on 9, 24)

/*	Configurations

	I2S0_MCLK;					pin28(4)	pin11(6)	

	I2S0_RX_BCLK:	pin11(4)	pin27(4)
	I2S0_RX_FS;		pin12(4)	pin29(4)
	I2S0_RDR0:		Pin13(4)
	I2S0_RDR1:		Pin30(4)

	I2S0_TX_FS;								pin23(6)
	I2S0_TX_BCLK:							pin9(6)
	I2S0_TDR0:								Pin22(6)
	I2S0_TDR1:								Pin15(6)
*/

void i2s_configurePorts(int iconf)
{
	if(iconf==1) //pjrc audio or wmxz cs5361 stereo board
	{
		CORE_PIN9_CONFIG  = PORT_PCR_MUX(6); // pin 9,  PTC3, I2S0_TX_BCLK
		CORE_PIN11_CONFIG = PORT_PCR_MUX(6); // pin 11, PTC6, I2S0_MCLK
		CORE_PIN23_CONFIG = PORT_PCR_MUX(6); // pin 23, PTC2, I2S0_TX_FS
		CORE_PIN13_CONFIG = PORT_PCR_MUX(4); // pin 13, PTC5, I2S0_RXD0
		CORE_PIN30_CONFIG = PORT_PCR_MUX(4); // pin 30, PTC11,I2S0_RXD1
	}
	else if(iconf==2) // pure rx system no MCLK
	{
		CORE_PIN11_CONFIG = PORT_PCR_MUX(4); // pin 11, PTC6, I2S0_RX_BCLK
		CORE_PIN12_CONFIG = PORT_PCR_MUX(4); // pin 12, PTC7, I2S0_RX_FS
		CORE_PIN13_CONFIG = PORT_PCR_MUX(4); // pin 13, PTC5, I2S0_RXD0
		CORE_PIN30_CONFIG = PORT_PCR_MUX(4); // pin 30, PTC11,I2S0_RXD1
	}
}

extern void i2sInProcessing(void * s, void * d)  __attribute__ ((weak));
extern void i2sOutProcessing(void * s, void * d) __attribute__ ((weak));

void m_i2s_tx_isr(void);
void m_i2s_rx_isr(void);
int8_t * m_i2s_tx_buffer;
int8_t * m_i2s_rx_buffer;
int m_i2s_tx_nbyte;
int m_i2s_rx_nbyte;

DMA_STRUCT *DMA_TX=0, *DMA_RX=0;

volatile static i2s_context_t m_i2s_txContext;// = { (NBUF/2)*(I2S_NBITS/8) , NSAMP, NCH};
volatile static i2s_context_t m_i2s_rxContext;// = { (NBUF/2)*(I2S_NBITS/8) , NSAMP, NCH};

void i2s_setupOutput(void * buffer, int ndat, int port, int prio)
{	// if transmitter is enabled, do nothing
	if (I2S0_TCSR & I2S_TCSR_TE) return;
	
	m_i2s_tx_buffer = buffer;
	m_i2s_tx_nbyte = ndat*m_i2s_nbits/8;
	
	m_i2s_txContext.nbytes=ndat/2*m_i2s_nbits/8;

	if(!DMA_TX) DMA_TX=DMA_allocate(port);
	if(!DMA_TX) return;
	
	if(m_i2s_dual & I2S_TX_2CH)
	{
		DMA_sourceBuffer_2ch(DMA_TX, buffer, ndat/2, m_i2s_nbits/8);
		DMA_destination_2ch(DMA_TX, (uint32_t *)&I2S0_TDR0, m_i2s_nbits/8);
		m_i2s_txContext.nsamp=ndat/2/4;
		m_i2s_txContext.nchan=4;
	}
	else
	{
		DMA_sourceBuffer(DMA_TX, buffer, ndat, m_i2s_nbits/8);
		DMA_destination(DMA_TX, (uint32_t *)&I2S0_TDR0, m_i2s_nbits/8);
		m_i2s_txContext.nsamp=ndat/2/2;
		m_i2s_txContext.nchan=2;
	}
	//
	DMA_interruptAtCompletion(DMA_TX); 
	DMA_interruptAtHalf(DMA_TX); 
	//
	DMA_attachInterrupt(DMA_TX, m_i2s_tx_isr); 
	DMA_triggerAtHardwareEvent(DMA_TX, DMAMUX_SOURCE_I2S0_TX) ;
	if(prio>0) NVIC_SET_PRIORITY(IRQ_I2S0_TX, prio*16); // 8 is normal priority (set in mk20dx128.c)
}

//#include "usb_serial.h"
//void logg(char c){	usb_serial_putchar(c);	usb_serial_flush_output();}

void i2s_setupInput(void * buffer, int ndat, int port, int prio)
{	// if receiver is enabled, do nothing
	if (I2S0_RCSR & I2S_RCSR_RE) return;
	
	m_i2s_rx_buffer=buffer;
	m_i2s_rx_nbyte = ndat*m_i2s_nbits/8;

	m_i2s_rxContext.nbytes=ndat/2*m_i2s_nbits/8;
	
	if(!DMA_RX) DMA_RX=DMA_allocate(port);
	if(!DMA_RX) return;
	//
	if(m_i2s_dual & I2S_RX_2CH)
	{ 	DMA_source_2ch(DMA_RX, (uint32_t *)&I2S0_RDR0, m_i2s_nbits/8);
		DMA_destinationBuffer_2ch(DMA_RX, buffer, ndat/2, m_i2s_nbits/8);
		m_i2s_rxContext.nsamp=ndat/2/4;
		m_i2s_rxContext.nchan=4;
	}
	else
	{ 	DMA_source(DMA_RX, (uint32_t *)&I2S0_RDR0, m_i2s_nbits/8);
		DMA_destinationBuffer(DMA_RX, buffer, ndat, m_i2s_nbits/8);
		m_i2s_rxContext.nsamp=ndat/2/2;
		m_i2s_rxContext.nchan=2;
	}
	//
	DMA_interruptAtCompletion(DMA_RX); 
	DMA_interruptAtHalf(DMA_RX); 
	//
	DMA_attachInterrupt(DMA_RX, m_i2s_rx_isr); 
	DMA_triggerAtHardwareEvent(DMA_RX, DMAMUX_SOURCE_I2S0_RX) ;
	if(prio>0) NVIC_SET_PRIORITY(IRQ_I2S0_RX, prio*16); // 8 is normal priority (set in mk20dx128.c)
	//
}

void i2s_stop(void)
{ //stops and resets Input
	if(DMA_TX) DMA_disable(DMA_TX);
	if(DMA_RX) DMA_disable(DMA_RX);
	I2S0_TCSR = 0;
	I2S0_RCSR = 0;
}

void i2s_startInput(void)
{	I2S0_RCSR |= I2S_RCSR_RE | I2S_RCSR_BCE;
}

void i2s_startOutput(void)
{	I2S0_TCSR |= I2S_TCSR_TE | I2S_TCSR_BCE; 
}

void i2s_enableInputDMA(void)
{	if(!DMA_RX) return;
	DMA_enable(DMA_RX);
	I2S0_RCSR |= I2S_RCSR_FRDE | I2S_RCSR_FR;
}

void i2s_enableOutputDMA(void)
{	if(!DMA_TX) return;
	DMA_enable(DMA_TX);
	I2S0_TCSR |= I2S_TCSR_FRDE | I2S_TCSR_FR;
}

void m_i2s_tx_isr(void)
{	uint32_t saddr, taddr;

	saddr = (uint32_t) DMA_sourceAddress(DMA_TX);
	DMA_clearInterrupt(DMA_TX);
	//
	if (saddr < (uint32_t)(&m_i2s_tx_buffer[m_i2s_tx_nbyte/2])) 
	{
		// DMA is sending from the first half of the buffer
//		digitalWriteFast(21, HIGH);
		// need to load data to the second half
		taddr=(uint32_t) &m_i2s_tx_buffer[m_i2s_tx_nbyte/2];
	} 
	else 
	{
		// DMA is sending from the second half of the buffer
//		digitalWriteFast(21, LOW);
		// need to load data to the first half
		taddr=(uint32_t) &m_i2s_tx_buffer[0];
	}
	//
	i2sOutProcessing((void *) &m_i2s_txContext,(void *) taddr);
}

void m_i2s_rx_isr(void)
{	uint32_t daddr, taddr;
	//
	daddr = (uint32_t) DMA_destinationAddress(DMA_RX);
	DMA_clearInterrupt(DMA_RX);
	//
	if (daddr < (uint32_t)(&m_i2s_rx_buffer[m_i2s_rx_nbyte/2])) 
	{
		// DMA is receiving to the first half of the buffer
		digitalWriteFast(20, HIGH);
		// need to process data from the second half
		taddr=(uint32_t) &m_i2s_rx_buffer[m_i2s_rx_nbyte/2];
	} 
	else 
	{
		// DMA is receiving to the second half of the buffer
		digitalWriteFast(20, LOW);
		// need to process data from the first half
		taddr=(uint32_t) &m_i2s_rx_buffer[0];
	}
	//
//	*(uint32_t*)(taddr+0)  = (uint32_t) &m_i2s_rx_buffer[0];
//	*(uint32_t*)(taddr+4)  = (uint32_t) &m_i2s_rx_buffer[m_i2s_rx_nbyte/2];
//	*(uint32_t*)(taddr+8)  = (uint32_t) &m_i2s_rx_buffer[m_i2s_rx_nbyte];
//	*(uint32_t*)(taddr+12)  = taddr;
	//
	//i2sInProcessing((void *) &m_i2s_rxContext,(void *) taddr);
	JOB_add((Fxn_t) i2sInProcessing, (void *) &m_i2s_rxContext,(void *) taddr,-1);
}

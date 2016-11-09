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
//spi.c

#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>
#include "usb_serial.h"

#include "kinetis.h"
#include "core_pins.h"

#include "dma.h"
#include "jobs.h"
#include "spi.h"
//
// DBR doubling boudrate (only in master mode)
// SCK baud rate = (fSYS/PBR) x [(1+DBR)/BR]
// BR =  [0:15] => [2+2*[0:2] 2^(3:15)] = [2,4,6, 8,16,...]
// PBR = [0:3]  => [2+0, 1+2*[1:3]] = [2,3,5,7]
// DBR = [0,1]
// define FULL_SPEED (<=20MHz)
#if F_BUS == 144000000
	#define SPI_CLOCK   (SPI_CTAR_PBR(3) | SPI_CTAR_DBR) //(144 / 7) * ((1+1)/2) = 10.55 MHz

#elif F_BUS == 72000000
	#define SPI_CLOCK   (SPI_CTAR_PBR(2) | SPI_CTAR_DBR) //(72 / 5) * ((1+1)/2) = 14.4 MHz

#elif F_BUS == 60000000
	#define SPI_CLOCK   (SPI_CTAR_PBR(1) | SPI_CTAR_DBR) //(60 / 3) * ((1+1)/2) = 20 MHz

#elif F_BUS == 56000000
	#define SPI_CLOCK   (SPI_CTAR_PBR(1) | SPI_CTAR_DBR) //(56 / 3) * ((1+1)/2) = 18.66 MHz

#elif F_BUS == 54000000
	#define SPI_CLOCK   (SPI_CTAR_PBR(1) | SPI_CTAR_DBR) //(56 / 3) * ((1+1)/2) = 18.66 MHz

#elif F_BUS == 48000000
	#define SPI_CLOCK   (SPI_CTAR_PBR(1) | SPI_CTAR_DBR) //(48 / 3) * ((1+1)/2) = 16 MHz

#elif F_BUS == 36000000
	#define SPI_CLOCK   (SPI_CTAR_PBR(0) | SPI_CTAR_DBR) //(36 / 2) * ((1+1)/2) = 18 MHz

#else
	#error "F_BUS must be 144, 72, 60, 56, 54, 48, 36 MHz"
#endif

#ifdef __MK20DX256__
KINETISK_SPI_t * SPI[] = {(KINETISK_SPI_t *)0x4002C000};

uint16_t DMAMUX_SOURCE_SPI_RX[] = {DMAMUX_SOURCE_SPI0_RX};
uint16_t DMAMUX_SOURCE_SPI_TX[] = {DMAMUX_SOURCE_SPI0_TX};
#endif
#ifdef __MK66FX1M0__
static KINETISK_SPI_t * SPI[] = {(KINETISK_SPI_t *)0x4002C000, (KINETISK_SPI_t *)0x4002D000, (KINETISK_SPI_t *)0x400AC000};

uint16_t DMAMUX_SOURCE_SPI_RX[] = {DMAMUX_SOURCE_SPI0_RX, DMAMUX_SOURCE_SPI1_RX, DMAMUX_SOURCE_SPI2_RX};
uint16_t DMAMUX_SOURCE_SPI_TX[] = {DMAMUX_SOURCE_SPI0_TX, DMAMUX_SOURCE_SPI1_TX, DMAMUX_SOURCE_SPI2_TX};
#endif




// following ctarTAB adapted from SPI
typedef struct {
	uint16_t div;
	uint32_t ctar;
} CTAR_TAB_t;

const CTAR_TAB_t ctarTabo[23] =
{
	{2, 	SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0) | SPI_CTAR_DBR},
	{3, 	SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0) | SPI_CTAR_DBR},
	{4, 	SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0)},
	{5, 	SPI_CTAR_PBR(2) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0) | SPI_CTAR_DBR},
	{6, 	SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0)},
	{8, 	SPI_CTAR_PBR(0) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1)},
	{10,	SPI_CTAR_PBR(2) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0)},
	{12,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1)},
	{16,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2)},
	{20,	SPI_CTAR_PBR(2) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(0)},
	{24,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2)},
	{32,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(4) | SPI_CTAR_CSSCK(3)},
	{40,	SPI_CTAR_PBR(2) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2)},
	{56,	SPI_CTAR_PBR(3) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2)},
	{64,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(5) | SPI_CTAR_CSSCK(4)},
	{96,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(5) | SPI_CTAR_CSSCK(4)},
	{128,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(6) | SPI_CTAR_CSSCK(5)},
	{192,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(6) | SPI_CTAR_CSSCK(5)},
	{256,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6)},
	{384,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6)},
	{512,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(8) | SPI_CTAR_CSSCK(7)},
	{640,	SPI_CTAR_PBR(2) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6)},
	{768,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(8) | SPI_CTAR_CSSCK(7)}
} ;


//============================ SPI ===================================
static DMAMEM uint32_t dummy  __attribute__( ( aligned ( 16 ) ) );
volatile static int m_spi_cs;

volatile static int m_spi_isAsserted=0;
int spi_isAsserted() {return m_spi_isAsserted;}

void spi_assertCS(int16_t port, int cs, uint32_t ctar)
{ 	if(cs>0) m_spi_cs=cs; 
	m_spi_isAsserted=1;
	spi_reserve(port, ctar);
	digitalWriteFast(m_spi_cs, LOW);
}
void spi_releaseCS(void) 
{	digitalWriteFast(m_spi_cs, HIGH); //spi_transfer(0xff);
	spi_release();
	m_spi_isAsserted=0;
}

void spi_init(void)
{
    SIM_SCGC6 |= SIM_SCGC6_SPI0;
}

void spi_configPorts(int iconf)
{
	if(iconf==1) // PJRC Audio
	{
		CORE_PIN14_CONFIG  = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE ; // CLK SRE slew rate enable, DSE drive strength enable 
		CORE_PIN7_CONFIG   = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE ; // MOSI
		CORE_PIN12_CONFIG  = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE;	 // MISO
	}
	else if(iconf==2) //WMXZ_Logger
	{
		CORE_PIN14_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE ; // CLK SRE slew rate enable, DSE drive strength enable 
		CORE_PIN7_CONFIG  = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE ;	// MOSI
		CORE_PIN8_CONFIG  = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE;	// MISO
	}
}

//#define DO_DEBUG1
uint32_t spi_setup(int16_t port, int div)
{	int ii;
	
	//spi SETUP	
	SPI[port]->MCR =	SPI_MCR_HALT |   // stop transfer
					SPI_MCR_PCSIS(0x1F) | // set all inactive states high
					SPI_MCR_CLR_TXF | // clear TXFIFO
					SPI_MCR_CLR_RXF | // clear RXFIFO
					SPI_MCR_MSTR;

//	uint32_t ctar = SPI_CLOCK | SPI_CTAR_BR(sp) | SPI_CTAR_CSSCK(sp?sp-1:0) | SPI_CTAR_FMSZ(7);
	for(ii=0; ii<23 && div > ctarTabo[ii].div; ii++) ;
	#ifdef DO_DEBUG
		static char text[80]; 
		sprintf(text,"%d %d %d\n\r",div,ii,ctarTab[ii].div); 
		usb_serial_write(text,80);
	#endif

	uint32_t ctar = ctarTabo[ii].ctar | SPI_CTAR_FMSZ(7);
//	ctar |= SPI_CTAR_CPHA;
	SPI[port]->CTAR0 =	ctar;
	SPI[port]->CTAR1 =	ctar | SPI_CTAR_FMSZ(8);
	
	SPI[port]->MCR = 	SPI_MCR_MSTR | SPI_MCR_DIS_TXF | SPI_MCR_DIS_RXF | SPI_MCR_PCSIS(0x1F);
	//
	return ctar;
}

//--------------------------------- used in uSD (from SDFAT)-----------------------------------
// use 16-bit frame if SPI_USE_8BIT_FRAME is zero
#define SPI_USE_8BIT_FRAME 1
// Limit initial fifo to three entries to avoid fifo overrun
#define SPI_INITIAL_FIFO_DEPTH 3

// define some symbols that are not in mk20dx128.h
#ifndef SPI_SR_RXCTR
	#define SPI_SR_RXCTR 0XF0
#endif  // SPI_SR_RXCTR

#ifndef SPI_PUSHR_CONT
	#define SPI_PUSHR_CONT 0X80000000
#endif   // SPI_PUSHR_CONT

#ifndef SPI_PUSHR_CTAS
	#define SPI_PUSHR_CTAS(n) (((n) & 7) << 28)
#endif  // SPI_PUSHR_CTAS

/** SPI receive a byte */
uint8_t spi_receive(int16_t port)
{
	SPI[port]->MCR |= SPI_MCR_CLR_RXF;
	SPI[port]->SR = SPI_SR_TCF;
	SPI[port]->PUSHR = 0xFF;
	while (!(SPI[port]->SR & SPI_SR_TCF)) {}
	return SPI[port]->POPR;
}
/** SPI send a byte */
void spi_send(int16_t port, uint8_t b)
{
	SPI[port]->MCR |= SPI_MCR_CLR_RXF;
	SPI[port]->SR = SPI_SR_TCF;
	SPI[port]->PUSHR = b;
	while (!(SPI[port]->SR & SPI_SR_TCF)) {}
}

/** SPI receive multiple bytes */
uint8_t spi_receiveBulk(int16_t port, uint8_t* buf, size_t n) {
  // clear any data in RX FIFO
	SPI[port]->MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);
  
#if SPI_USE_8BIT_FRAME
  // initial number of bytes to push into TX FIFO
  int nf = n < SPI_INITIAL_FIFO_DEPTH ? n : SPI_INITIAL_FIFO_DEPTH;
  int ii;
  for (ii = 0; ii < nf; ii++) {
	  SPI[port]->PUSHR = 0XFF;
  }
  // limit for pushing dummy data into TX FIFO
  uint8_t* limit = buf + n - nf;
  while (buf < limit) {
    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
    SPI[port]->PUSHR = 0XFF;
    *buf++ = SPI[port]->POPR;
  }
  // limit for rest of RX data
  limit += nf;
  while (buf < limit) {
    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
    *buf++ = SPI[port]->POPR;
  }
  
#else  // SPI_USE_8BIT_FRAME
  // use 16 bit frame to avoid TD delay between frames
  // get one byte if n is odd
  if (n & 1) {
    *buf++ = spi_receive();
    n--;
  }
  // initial number of words to push into TX FIFO
  int nf = n/2 < SPI_INITIAL_FIFO_DEPTH ? n/2 : SPI_INITIAL_FIFO_DEPTH;
  for (int i = 0; i < nf; i++) {
	  SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | 0XFFFF;
  }
  uint8_t* limit = buf + n - 2*nf;
  while (buf < limit) {
    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
    SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | 0XFFFF;
    uint16_t w = SPI[port]->POPR;
    *buf++ = w >> 8;
    *buf++ = w & 0XFF;
  }
  // limit for rest of RX data
  limit += 2*nf;
  while (buf < limit) {
    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
    uint16_t w = SPI[port]->POPR;
    *buf++ = w >> 8;
    *buf++ = w & 0XFF;
  }
#endif  // SPI_USE_8BIT_FRAME
  return 0;
}

/** SPI send multiple bytes */
void spi_sendBulk(int16_t port, const uint8_t* buf , size_t n) {
  // clear any data in RX FIFO
	SPI[port]->MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);

#if SPI_USE_8BIT_FRAME
  // initial number of bytes to push into TX FIFO
  int nf = n < SPI_INITIAL_FIFO_DEPTH ? n : SPI_INITIAL_FIFO_DEPTH;
  // limit for pushing data into TX fifo
  const uint8_t* limit = buf + n;
  int ii;
  for (ii = 0; ii < nf; ii++) {
	  SPI[port]->PUSHR = *buf++;
  }
  // write data to TX FIFO
  while (buf < limit) {
    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
    SPI[port]->PUSHR = *buf++;
    SPI[port]->POPR;
  }
  // wait for data to be sent
  while (nf) {
    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
    SPI[port]->POPR;
    nf--;
  }

#else  // SPI_USE_8BIT_FRAME
  // use 16 bit frame to avoid TD delay between frames
  // send one byte if n is odd
  if (n & 1) {
    spi_send(*buf++);
    n--;
  }
  // initial number of words to push into TX FIFO
  int nf = n/2 < SPI_INITIAL_FIFO_DEPTH ? n/2 : SPI_INITIAL_FIFO_DEPTH;
  // limit for pushing data into TX fifo
  const uint8_t* limit = buf + n;
  int ii;
  for (ii = 0; ii < nf; ii++) {
    uint16_t w = (*buf++) << 8;
    w |= *buf++;
    SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | w;
  }
  // write data to TX FIFO
  while (buf < limit) {
    uint16_t w = *buf++ << 8;
    w |= *buf++;
    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
    SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | w;
    SPI[port]->POPR;
  }
  // wait for data to be sent
  while (nf) {
    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
    SPI[port]->POPR;
    nf--;
  }
#endif  // SPI_USE_8BIT_FRAME
}

//--------------------------------------------------------------------

uint8_t spi_transfer(int16_t port, uint8_t data)
{
	SPI[port]->SR = SPI_SR_TCF;
	SPI[port]->PUSHR = data;
	while (!(SPI[port]->SR & SPI_SR_TCF)) ; // wait
	return SPI[port]->POPR;
}
uint16_t spi_transfer16(int16_t port, uint16_t data)
{
	SPI[port]->SR = SPI_SR_TCF;
	SPI[port]->PUSHR = data | SPI_PUSHR_CTAS(1);
	while (!(SPI[port]->SR & SPI_SR_TCF)) ; // wait
	return SPI[port]->POPR;
}

void spi_transferBlock(int16_t port, void *inpbuf, void *outbuf, size_t count)
{	int ii;
		if (count == 0) return;
		uint8_t *inp=(uint8_t *)inpbuf;
		uint8_t *out=(uint8_t *)outbuf;
		
		SPI[port]->RSER = 0; // no interrupts
		//
	    // clear any data in RX FIFO
		SPI[port]->MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);

	    // initial number of bytes to push into TX FIFO
	    int nf = count < SPI_INITIAL_FIFO_DEPTH ? count : SPI_INITIAL_FIFO_DEPTH;
	  
	    for (ii = 0; ii < nf; ii++) 
	    {
	    	SPI[port]->PUSHR = *out++;
	    }
	    // limit for pushing dummy data into TX FIFO
	    uint8_t* limit = (uint8_t* )out + count - nf;
	    while ((uint8_t* )out < limit) 
		{
		   while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
		   SPI[port]->PUSHR = *out++;
		   *inp++ = SPI[port]->POPR;
	    }
	    // limit for rest of RX data
	    limit += nf;
	    while ((uint8_t* )inp < limit) 
		{
		  while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
		  *inp++ = SPI[port]->POPR;
        }
}

void spi_transferBlock16(int16_t port, void *inpbuf, void *outbuf, size_t count)
{ 	int ii;
	uint16_t *inp=(uint16_t *)inpbuf;
	uint16_t *out=(uint16_t *)outbuf;

	SPI[port]->RSER = 0; // no interrupts
	//
	// clear any data in RX FIFO
	SPI[port]->MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);
	// use 16 bit frame to avoid TD delay between frames
	
	// initial number of words to push into TX FIFO
	int nf = count/2 < SPI_INITIAL_FIFO_DEPTH ? count/2 : SPI_INITIAL_FIFO_DEPTH;
	for (ii = 0; ii < nf; ii++) 
	{
		SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | *out++;
	}
	uint8_t* limit = (uint8_t*)out + count - 2*nf;
	while ((uint8_t* )out < limit) 
	{
		while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
		SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | *out++;
		uint16_t w = SPI[port]->POPR;
		*inp++ = w >> 8;
		*inp++ = w & 0XFF;
	}
	// limit for rest of RX data
	limit += 2*nf;
	while ((uint8_t* )inp < limit) 
	{
		while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
		uint16_t w = SPI[port]->POPR;
		*inp++ = w >> 8;
		*inp++ = w & 0XFF;
	}
}

void spi_read16(int16_t port, uint8_t *buf, size_t len)
{  if (len == 0) return;

	//align with 8 BIT Transfer
	if ((uintptr_t)buf & 0x01) {	*buf++ = spi_transfer(port, 0);	len--;	}
	//16BIT Transfer
	while(len > 1) {	*(uint16_t*)buf = spi_transfer16(port, 0);	buf += 2;	len -= 2;}
	//8BIT Transfer
	while(len > 0) {	*buf++ = spi_transfer(port, 0);	len--;	}
	spi_releaseCS();
}

void spi_write16(int16_t port, uint8_t *buf, size_t len)
{  if (len == 0) return;

   //align with 8 BIT Transfer
	if ((uintptr_t)buf & 0x01) {spi_transfer(port, *buf++); len--;}
	//16BIT Transfer
	while(len > 1) {spi_transfer16(port, *(uint16_t*)buf);buf += 2;	len -= 2;}
	//8BIT Transfer
	while(len > 0) {spi_transfer(port, *buf++);	len--;	}
	spi_releaseCS();
}

//waitFifoEmpty taken from ILI9341_t3.h, (c) Paul Stoffregen ( and used by FrankB)
void spi_waitFifoEmpty(int16_t port)
{
	uint32_t sr;
	uint32_t tmp __attribute__((unused));
	do {
		sr = SPI[port]->SR;
		if (sr & 0xF0) tmp = SPI[port]->POPR;	// drain RX FIFO
	} while ((sr & 0xF0F0) > 0);       	// wait both RX & TX empty 0xF0F0

	//clear all SR
//	SPI[port]->SR = 0xFF0F0000;
//	SPI[port]->SR = 0xDA0A0000;
}

void spi_drainRXFifo(int16_t port)
{	uint32_t tmp __attribute__((unused));
	while(SPI[port]->SR & SPI_SR_RFDF) tmp=SPI[port]->POPR;
}

void spi_flushTXFifo(void)
{
}
//============================ DMA ===================================
DMA_STRUCT *m_DMA_SPI_TX=0, *m_DMA_SPI_RX=0;
int m_dma_done=1;
typedef struct
{ 	Fxn_t f;
	Ptr s,d;
	int nice;
}	dma_CB_t;

dma_CB_t m_dma_CB={0,0,0,0};

int spi_dma_done(void) {return m_dma_done;}
void spi_dma_setCB(Fxn_t f, Ptr s, Ptr d, int nice) 
{	m_dma_CB.f = f; 
	m_dma_CB.s = s; 
	m_dma_CB.d = d; 
	m_dma_CB.nice=nice;
}

void m_DMA_SPI_RX_ISR(void)
{//	 __disable_irq();

	DMA_clearInterrupt(m_DMA_SPI_RX);
	
	while(!DMA_complete(m_DMA_SPI_RX)); // nexcessary?
	DMA_clearComplete(m_DMA_SPI_RX); 
	while(!DMA_complete(m_DMA_SPI_TX)); // nexcessary?
	DMA_clearComplete(m_DMA_SPI_TX); 
	//
	m_dma_done=1;
	spi_releaseCS();
	//
	if(m_dma_CB.f)
//		m_dma_CB_f(m_dma_CB_s,m_dma_CB_d);
	JOB_add((Fxn_t) m_dma_CB.f, (Ptr) m_dma_CB.s, (Ptr) m_dma_CB.s, m_dma_CB.nice);

// 	__enable_irq();
}

int spi_dmaSetup(int16_t port, int dma_tx, int dma_rx, int prio_rx)
{
	// TX DMA
	m_DMA_SPI_TX=DMA_allocate(dma_tx); if(!m_DMA_SPI_TX) return 0;
	DMA_disable(m_DMA_SPI_TX);
	DMA_destination(m_DMA_SPI_TX, (uint32_t *) &SPI[port]->PUSHR, sizeof(int8_t));
	DMA_triggerAtHardwareEvent(m_DMA_SPI_TX, DMAMUX_SOURCE_SPI_TX[port]) ;
	DMA_disableOnCompletion(m_DMA_SPI_TX);

	// RX DMA
	m_DMA_SPI_RX=DMA_allocate(dma_rx); if(!m_DMA_SPI_RX) return 0;
	DMA_disable(m_DMA_SPI_RX);
	DMA_source(m_DMA_SPI_RX, (uint32_t *) &SPI[port]->POPR, sizeof(int8_t));
	DMA_triggerAtHardwareEvent(m_DMA_SPI_RX, DMAMUX_SOURCE_SPI_RX[port]) ;
	DMA_disableOnCompletion(m_DMA_SPI_RX);

	// interrupt only on RX DMA
	NVIC_SET_PRIORITY(IRQ_DMA_CH0+m_DMA_SPI_RX->channel,prio_rx*16);
	DMA_attachInterrupt(m_DMA_SPI_RX, m_DMA_SPI_RX_ISR); 
	DMA_interruptAtCompletion(m_DMA_SPI_RX); 
		
//	DMA_startAll();
	return 1;
}

void spi_readDMA_noWait(int16_t port, int8_t *buf,  size_t len)
{ 
// set transmit
	DMA_source(m_DMA_SPI_TX,(void *)&dummy, sizeof(int8_t));
	DMA_transferCount(m_DMA_SPI_TX, len);

// set receive
	DMA_destinationBuffer(m_DMA_SPI_RX, (void *) buf, len, sizeof(int8_t));

	m_dma_done=0;
	spi_waitFifoEmpty(port);
	
	SPI[port]->SR = 0xFF0F0000;
	SPI[port]->MCR |= SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF;
	SPI[port]->RSER = SPI_RSER_TFFF_RE | SPI_RSER_TFFF_DIRS | SPI_RSER_RFDF_RE | SPI_RSER_RFDF_DIRS;

	DMA_clearComplete(m_DMA_SPI_TX); // clear here before starting?
	DMA_enable(m_DMA_SPI_RX);
	DMA_enable(m_DMA_SPI_TX);
}

void spi_writeDMA_noWait(int16_t port, int8_t *buf,  size_t len)
{

// set transmit
	DMA_sourceBuffer(m_DMA_SPI_TX,(void *)buf, len, sizeof(int8_t));

// set receive
	DMA_destination(m_DMA_SPI_RX,(void *)&dummy, sizeof(int8_t));
	DMA_transferCount(m_DMA_SPI_RX, len);
	
	m_dma_done=0;
	spi_waitFifoEmpty(port);
	
//	SPI[port]->MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF;
//	SPI[port]->MCR |= SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF;
	SPI[port]->RSER = SPI_RSER_TFFF_RE | SPI_RSER_TFFF_DIRS | SPI_RSER_RFDF_RE | SPI_RSER_RFDF_DIRS;

	DMA_clearComplete(m_DMA_SPI_TX);  //clear here?
	DMA_enable(m_DMA_SPI_RX);
	DMA_enable(m_DMA_SPI_TX);
}

// adapted from SPI transactions
volatile static uint8_t  m_spi_interruptMasksUsed=0;
volatile static uint32_t m_spi_interruptMask[(NVIC_NUM_INTERRUPTS+31)/32];
volatile static uint32_t m_spi_interruptSave[(NVIC_NUM_INTERRUPTS+31)/32];

// following is for interrupts via digital pins
void spi_usingPinInterrupt(uint8_t n)
{
	if (n == 3 || n == 4 || n == 24 || n == 33) 
	{	spi_usingInterrupt(IRQ_PORTA);
	} 
	else if (n == 0 || n == 1 || (n >= 16 && n <= 19) || n == 25 || n == 32) 
	{	spi_usingInterrupt(IRQ_PORTB);
	} 
	else if ((n >= 9 && n <= 13) || n == 15 || n == 22 || n == 23
	  || (n >= 27 && n <= 30)) 
	{	spi_usingInterrupt(IRQ_PORTC);
	} 
	else if (n == 2 || (n >= 5 && n <= 8) || n == 14 || n == 20 || n == 21) 
	{	spi_usingInterrupt(IRQ_PORTD);
	} 
	else if (n == 26 || n == 31) 
	{	spi_usingInterrupt(IRQ_PORTE);
	}
}

// use IRQ_NUMBER defined in kinietis.h
void spi_usingInterrupt(IRQ_NUMBER nirq)
{
	uint32_t n = (uint32_t)nirq;
	if (n >= NVIC_NUM_INTERRUPTS) return;

	m_spi_interruptMasksUsed |= (1 << (n >> 5));
	m_spi_interruptMask[n >> 5] |= (1 << (n & 0x1F));
}

void spi_notUsingInterrupt(IRQ_NUMBER nirq)
{
	uint32_t n = (uint32_t)nirq;
	if (n >= NVIC_NUM_INTERRUPTS) return;
	
	m_spi_interruptMask[n >> 5] &= ~(1 << (n & 0x1F));
	if (m_spi_interruptMask[n >> 5] == 0) 
	{
		m_spi_interruptMasksUsed &= ~(1 << (n >> 5));
	}
}

// from  beginTransactions
void spi_reserve(int16_t port, uint32_t ctar)
{
	if (m_spi_interruptMasksUsed) 
	{
		__disable_irq();
		if (m_spi_interruptMasksUsed & 0x01) 
		{
			m_spi_interruptSave[0] = NVIC_ICER0 & m_spi_interruptMask[0];
			NVIC_ICER0 = m_spi_interruptSave[0];
		}
		#if NVIC_NUM_INTERRUPTS > 32
		if (m_spi_interruptMasksUsed & 0x02) 
		{
			m_spi_interruptSave[1] = NVIC_ICER1 & m_spi_interruptMask[1];
			NVIC_ICER1 = m_spi_interruptSave[1];
		}
		#endif
		#if NVIC_NUM_INTERRUPTS > 64 && defined(NVIC_ISER2)
		if (m_spi_interruptMasksUsed & 0x04) 
		{
			m_spi_interruptSave[2] = NVIC_ICER2 & m_spi_interruptMask[2];
			NVIC_ICER2 = m_spi_interruptSave[2];
		}
		#endif
		#if NVIC_NUM_INTERRUPTS > 96 && defined(NVIC_ISER3)
		if (m_spi_interruptMasksUsed & 0x08) 
		{
			m_spi_interruptSave[3] = NVIC_ICER3 & m_spi_interruptMask[3];
			NVIC_ICER3 = m_spi_interruptSave[3];
		}
		#endif
		__enable_irq();
	}
	
	if (SPI[port]->CTAR0 != ctar)
	{
		SPI[port]->MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);
		SPI[port]->CTAR0 = ctar;
		SPI[port]->CTAR1 = ctar | SPI_CTAR_FMSZ(8);
		SPI[port]->MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F);
	}
}

 // from endTransactions
 void spi_release(void)
{	if (m_spi_interruptMasksUsed) 
	{
		if (m_spi_interruptMasksUsed & 0x01) 
		{
			NVIC_ISER0 = m_spi_interruptSave[0];
		}
		#if NVIC_NUM_INTERRUPTS > 32
		if (m_spi_interruptMasksUsed & 0x02) 
		{
			NVIC_ISER1 = m_spi_interruptSave[1];
		}
		#endif
		#if NVIC_NUM_INTERRUPTS > 64 && defined(NVIC_ISER2)
		if (m_spi_interruptMasksUsed & 0x04) 
		{
			NVIC_ISER2 = m_spi_interruptSave[2];
		}
		#endif
		#if NVIC_NUM_INTERRUPTS > 96 && defined(NVIC_ISER3)
		if (m_spi_interruptMasksUsed & 0x08) 
		{
			NVIC_ISER3 = m_spi_interruptSave[3];
		}
		#endif
	}
}

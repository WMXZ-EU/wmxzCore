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
// 20-may-17: added ICS43432
// 04-jun-17: modified speed parameter estimation

#include "kinetis.h"
#include "core_pins.h"

#include "i2s.h"
#include "dma.h"

//#define HAVE_HW_SERIAL
#ifdef HAVE_HW_SERIAL

#include "localKinetis.h"

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
#ifdef USE_I2S_PIN
	pinMode(I2S_PIN,OUTPUT);
#endif
}

int iscl[3];

#define NPRIMES 46
int64_t primes[NPRIMES]={2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 
					31, 37, 41, 43, 47, 53, 59, 61,	67, 71, 
					73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
					127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
					179, 181, 191, 193, 197, 199};

float i2s_speedConfig(int device, int nbits, int fs)
{
// rules to generate click dividers
//  MCGPLLCLK=F_CPU // is set by _MICS(3)
//  MCLK = MCGPLLCLK*(iscl1+1)/(iscl2+1)
//	BCLK = MCLK/2/(iscl3+1)
//  LRCLK = BCLK/(2*nbits); // division by  is to have 32 bits within frame sync (BCLK)
//
	int ii;
	int64_t i1,i2,i3;
	//
	if((device==SGTL5000_DEV) || (device==PJRC_AUDIO_DEV) || (device==CS5361_DEV))
	{
		int64_t nov;
		if (device==PJRC_AUDIO_DEV) 
		{	nov=256;
			nbits=12;
			fs=44100; // is fixed in Audio tool
		}

		else if (device==SGTL5000_DEV)
		{	nov=512;
			if(fs>48000) nov=256;
		}

		else if (device==CS5361_DEV)
		{   nbits = 32; 
			int64_t sc = 96000/fs;
			if(sc>2) sc=2;
			nov=256<<sc;
		}			
		
		// find reference frequency for rounding
		int64_t fref = 1000000; // start with 1 MHz
		while( (((fref/fs) % 8)>0) &&  (fref < F_CPU)) fref+= 1000000; 
		int64_t scl = fref/fs; // should now be multiple of 8
		//
		// find first multiplier
		int64_t bitRate = fref*nov;
		int64_t scale0 = F_CPU*scl;
		
		for(i1=1; i1<256;i1++) if ((scale0*i1 % bitRate)==0) break;
		if(i1==256) return 0.0f; // failed to find multiplier
		
		i2=scale0*i1 / bitRate;
		i3 = nov / (4*nbits);
		
		iscl[0] = (int) (i1-1);
		iscl[1] = (int) (i2-1);
		iscl[2] = (int) (i3-1);
	}
	
	else
  {   // find first multiplier
	int64_t bitRate = 2l*nbits*fs;
	int64_t scale0 = F_CPU/2;

	i1=1;
	while(scale0*i1 % bitRate) { i1++; if(i1==256) break;}
	//
	if(i1==256) return 0.0f; // failed to find multiplier

	int64_t scale1=scale0*i1/bitRate;
	i2=1;
	i3=1;
	while(1)
	{ 	for(ii=0;ii<NPRIMES;ii++)
			if ((scale1 % primes[ii])==0) { i2 *= primes[ii]; scale1/=primes[ii]; break; }
		if( scale1<=1) break;
		for(ii=0;ii<NPRIMES;ii++)
			if ((scale1 % primes[ii])==0) { i3 *= primes[ii]; scale1/=primes[ii]; break; }
		if( scale1<=1) break;
	}

	iscl[0]= (int) (i1-1);
	if(i2>i3) // take larger divider first
	{
		iscl[1] = (int) (i2-1);
		iscl[2] = (int) (i3-1);
	}
	else
	{
		iscl[1] = (int) (i3-1);
		iscl[2] = (int) (i2-1);
	}

	}
	return F_CPU * (float)(i1) / (float)(i2) / 2.0f / (float)(i3) / (float)(2*nbits); // is sampling frequency
}

void i2s_config(int isMaster, int nbits, int dual, int sync)
{
	int mcr_src;
#if defined(__MK20DX256__)
		mcr_src=3;
#elif defined(__MK66FX1M0__)
		mcr_src=0;
#endif

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
		I2S0_MCR = I2S_MCR_MICS(mcr_src)  | I2S_MCR_MOE;
		while (I2S0_MCR & I2S_MCR_DUF) ; 
		I2S0_MDR = I2S_MDR_FRACT(iscl[0]) | I2S_MDR_DIVIDE(iscl[1]); 
	}

	// configure transmitter
	I2S0_TMR = 0;
	I2S0_TCR1 = I2S_TCR1_TFW(1);  // watermark at half fifo size
	I2S0_TCR2 = I2S_TCR2_SYNC((1-sync)) | I2S_TCR2_BCP ; // sync=1; tx is async; rx = sync
	if(isMaster)
		I2S0_TCR2 |= (I2S_TCR2_BCD | I2S_TCR2_DIV(iscl[2]) | I2S_TCR2_MSEL(1));
	//
	if(dual & I2S_TX_2CH)
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
	I2S0_RCR2 = I2S_RCR2_SYNC(sync);// | I2S_RCR2_BCP ; // sync=0; rx is async; tx is sync
	if(isMaster)
		I2S0_RCR2 = (I2S_RCR2_BCD | I2S_RCR2_DIV(iscl[2]) | I2S_RCR2_MSEL(1));
	//
	if(dual & I2S_RX_2CH)
		I2S0_RCR3 = I2S_RCR3_RCE_2CH; // dual rx channel
	else
		I2S0_RCR3 = I2S_RCR3_RCE; // single rx channel
	//
	I2S0_RCR4 = I2S_RCR4_FRSZ(1) 
				| I2S_RCR4_SYWD((nbits-1)) 
				| I2S_RCR4_MF
				| I2S_RCR4_FSE	// frame sync early
				| I2S_RCR4_FSP	// sample at active low
				;
	if(isMaster)
		I2S0_RCR4 |= I2S_RCR4_FSD;	

	I2S0_RCR5 = I2S_RCR5_WNW((nbits-1)) | I2S_RCR5_W0W((nbits-1)) | I2S_RCR5_FBT((nbits-1));
	//
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
//      30/38   I2S0_RX_RDR1
//      32      I2S0_TX_BCLK                        (also on 9, 24)

/*	Configurations

	I2S0_MCLK;					pin28(4)	pin11(6)	

	I2S0_RX_BCLK:	pin11(4)	pin27(4)
	I2S0_RX_FS;		pin12(4)	pin29(4)
	I2S0_RDR0:		Pin13(4)
	I2S0_RDR1:		Pin30/38(4)

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
#ifdef __MK20DX256__
		CORE_PIN30_CONFIG = PORT_PCR_MUX(4); // pin 30, PTC11,I2S0_RXD1
#endif
#ifdef __MK66FX1M0__
		CORE_PIN38_CONFIG = PORT_PCR_MUX(4); // pin 38, PTC11,I2S0_RXD1
#endif
	}
	else if(iconf==2) // pure rx system no MCLK
	{
		CORE_PIN11_CONFIG = PORT_PCR_MUX(4); // pin 11, PTC6, I2S0_RX_BCLK
		CORE_PIN12_CONFIG = PORT_PCR_MUX(4); // pin 12, PTC7, I2S0_RX_FS
		CORE_PIN13_CONFIG = PORT_PCR_MUX(4); // pin 13, PTC5, I2S0_RXD0
#ifdef __MK20DX256__
		CORE_PIN30_CONFIG = PORT_PCR_MUX(4); // pin 30, PTC11,I2S0_RXD1
#endif
#ifdef __MK66FX1M0__
		CORE_PIN38_CONFIG = PORT_PCR_MUX(4); // pin 38, PTC11,I2S0_RXD1
#endif

	}
}

static void i2sDefaultRxIsr(void *s, void *d){}
static void i2sDefaultTxIsr(void *s, void *d){}
void i2sInProcessing(void * s, void * d)  __attribute__ ((weak, alias("i2sDefaultRxIsr")));
void i2sOutProcessing(void * s, void * d) __attribute__ ((weak, alias("i2sDefaultTxIsr")));

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
		m_i2s_txContext.nsamp=ndat/2/4;	// half buffer / 4-chan
		m_i2s_txContext.nchan=4;
	}
	else
	{
		DMA_sourceBuffer(DMA_TX, buffer, ndat, m_i2s_nbits/8);
		DMA_destination(DMA_TX, (uint32_t *)&I2S0_TDR0, m_i2s_nbits/8);
		m_i2s_txContext.nsamp=ndat/2/2;	// half buffer / 2-chan
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
{	// ndat is number of words in (dual) input buffer
	// if receiver is enabled, do nothing
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
		m_i2s_rxContext.nsamp=ndat/2/4; // half buffer/ 4-chan
		m_i2s_rxContext.nchan=4;
	}
	else
	{ 	DMA_source(DMA_RX, (uint32_t *)&I2S0_RDR0, m_i2s_nbits/8);
		DMA_destinationBuffer(DMA_RX, buffer, ndat, m_i2s_nbits/8);
		m_i2s_rxContext.nsamp=ndat/2/2;	// half buffer / 2-chan
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

	DMA_clearInterrupt(DMA_TX);
	saddr = (uint32_t) DMA_sourceAddress(DMA_TX);
	//
	if (saddr < (uint32_t)(&m_i2s_tx_buffer[m_i2s_tx_nbyte/2])) 
	{
		// DMA is sending from the first half of the buffer
		// need to load data to the second half
		taddr=(uint32_t) &m_i2s_tx_buffer[m_i2s_tx_nbyte/2];
	} 
	else 
	{
		// DMA is sending from the second half of the buffer
		// need to load data to the first half
		taddr=(uint32_t) &m_i2s_tx_buffer[0];
	}
	//
	i2sOutProcessing((void *) &m_i2s_txContext,(void *) taddr);
	//JOB_add((Fxn_t) i2sOutProcessing, (void *) &m_i2s_txContext,(void *) taddr,-1);
}

uint32_t i2sDma_getRxError(void) { return *DMA_RX->ES;}

volatile uint32_t rxCount = 0;
void m_i2s_rx_isr(void)
{	uint32_t daddr, taddr;
	//
	rxCount++;
	__disable_irq();
	DMA_clearInterrupt(DMA_RX);
	daddr = (uint32_t) DMA_destinationAddress(DMA_RX);
	//
	if (daddr < (uint32_t)(&m_i2s_rx_buffer[m_i2s_rx_nbyte/2])) 
	{
		// DMA is receiving to the first half of the buffer
#ifdef USE_I2S_PIN
		digitalWriteFast(I2S_PIN, HIGH);
#endif
		// need to process data from the second half
		taddr=(uint32_t) &m_i2s_rx_buffer[m_i2s_rx_nbyte/2];
	} 
	else 
	{
		// DMA is receiving to the second half of the buffer
#ifdef USE_I2S_PIN
		digitalWriteFast(I2S_PIN, LOW);
#endif
		// need to process data from the first half
		taddr=(uint32_t) &m_i2s_rx_buffer[0];
	}
	//
	i2sInProcessing((void *) &m_i2s_rxContext,(void *) taddr);
	//JOB_add((Fxn_t) i2sInProcessing, (void *) &m_i2s_rxContext,(void *) taddr,-1);
	__enable_irq();
}


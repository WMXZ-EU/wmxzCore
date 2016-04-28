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
//dma.c

// WMXZ
// Inspired by PJRC's DMAChannel.cpp
// converted to plain c
// changed slightly API

#include "stdbool.h"
#include "kinetis.h"
#include "core_pins.h"

#include "basics.h"
#include "dma.h"

#define DMA_DCHPRI_BASE 0x40008100

static uint16_t dma_channel_allocated_mask = 0;
static DMA_STRUCT DMAS[DMA_NUM_CHANNELS];

void DMA_init(void)
{
	SIM_SCGC6 |= SIM_SCGC6_DMAMUX;
	SIM_SCGC7 |= SIM_SCGC7_DMA;

	DMA_CR = DMA_CR_HALT | DMA_CR_EMLM; // fixed priority arbitration
	DMA_CR |= DMA_CR_ERCA; // enable round robin arbitration  // good idea?
}

uint32_t DMA_priority(int ch) 
{ 	uint32_t n; 
	n = *(uint32_t *)((uint32_t)&DMA_DCHPRI3 + (ch & 0xFC)); 
	n = __builtin_bswap32(n); 
	return (n >> ((ch & 0x03) << 3)) & 0x0F; 
} 

static int DMA_getChannel(int prio)
{	int ch;
	//find proper DMA according to its default priority (0 is lowest 15 is highest priority)
	ch= 4*(prio/4)+(3-(prio %4));
	
	//check if allready allocated
	if(((dma_channel_allocated_mask & (1<<ch)))) 
		return -1; //no channel found
	else
		SET_BIT(dma_channel_allocated_mask,(1<<ch));
	
	uint8_t *DCHPRI=(uint8_t *)(DMA_DCHPRI_BASE + (ch & 0x0F));
	if((*DCHPRI & 0x0F) != prio) return -1;

	*DCHPRI |= DMA_DCHPRI_ECP; // allow to be interrupted
	return ch;
}

void DMA_print(DMA_STRUCT *dma)
{// 	vectorPrint16((uint16_t*) dma->TCD,sizeof(DMA_TCD)/2,16,1,',');
}

#define DMA_TCD_BASE 0x40009000 
DMA_STRUCT *DMA_allocate(int prio)
{	int ch;
	ch=DMA_getChannel(prio);
	if(ch<0) return (DMA_STRUCT *)0;
	
	DMAS[ch].channel=ch;
	DMAS[ch].TCD = (DMA_TCD *)(DMA_TCD_BASE + ch * 32);
	DMAS[ch].TCD->SADDR=0;
	DMAS[ch].TCD->SOFF=0;
	DMAS[ch].TCD->ATTR=0;
	DMAS[ch].TCD->NBYTES=0;
	DMAS[ch].TCD->SLAST=0;
	DMAS[ch].TCD->DADDR=0;
	DMAS[ch].TCD->DOFF=0;
	DMAS[ch].TCD->CITER=0;
	DMAS[ch].TCD->DLAST_SGA=0;
	DMAS[ch].TCD->CSR=0;
	DMAS[ch].TCD->BITER=0;
	
	DMA_CDNE = ch;
	return &DMAS[ch];
}

void DMA_release(DMA_STRUCT *dma)
{	int ch;
	ch=dma->channel;
	CLEAR_BIT(dma_channel_allocated_mask,(1<<ch));
}

void DMA_source(DMA_STRUCT *dma, void *p, unsigned int wordsize)
{ 	DMA_TCD *TCD=dma->TCD;

	if(!((wordsize==1)||(wordsize==2)||(wordsize==4))) return; // limit to 1,2,4
	TCD->SADDR = p;
	TCD->SOFF = 0;
	TCD->ATTR |= DMA_TCD_ATTR_SSIZE(wordsize/2);
	TCD->NBYTES = wordsize;
	TCD->SLAST = 0;
}

void DMA_source_2ch(DMA_STRUCT *dma, void *p, unsigned int wordsize)
{ 	DMA_TCD *TCD=dma->TCD;

	if(!((wordsize==1)||(wordsize==2)||(wordsize==4))) return; // limit to 1,2,4
	TCD->SADDR = p;
	TCD->SOFF = 4;
//	TCD->ATTR |= wordsize/2 ;
//	TCD->NBYTES = DMA_TCD_NBYTES_SMLOE |
//					DMA_TCD_NBYTES_MLOFFYES_MLOFF(-8) |
//					DMA_TCD_NBYTES_MLOFFYES_NBYTES(2*wordsize);
//	TCD->SLAST = -8;
	TCD->ATTR |= DMA_TCD_ATTR_SSIZE(wordsize/2) | DMA_TCD_ATTR_SMOD(3);
	TCD->NBYTES = 2*wordsize;
	TCD->SLAST = 0;
}

void DMA_sourceBuffer(DMA_STRUCT *dma, void *p, unsigned int len, unsigned int wordsize) 
{	DMA_TCD *TCD=dma->TCD;

	if(!((wordsize==1)||(wordsize==2)||(wordsize==4))) return; // limit to 1,2,4
	TCD->SADDR = p;
	TCD->SOFF = wordsize;
	TCD->ATTR |= DMA_TCD_ATTR_SSIZE(wordsize/2);
//	TCD->NBYTES = wordsize;
	TCD->SLAST = -wordsize*len; // is in bytes
	TCD->BITER = TCD->CITER = len ; // is in samples
}

void DMA_sourceBuffer_2ch(DMA_STRUCT *dma, void *p, unsigned int len, unsigned int wordsize) 
{ 	DMA_TCD *TCD=dma->TCD;

	if(!((wordsize==1)||(wordsize==2)||(wordsize==4))) return; // limit to 1,2,4
	TCD->DADDR = p;
	TCD->DOFF = wordsize;
	TCD->ATTR |= DMA_TCD_ATTR_SSIZE(wordsize/2);
//	TCD->NBYTES = wordsize;
	TCD->SLAST = -wordsize*2*len; // is in bytes
	TCD->BITER = TCD->CITER = len; // number of samples
}

void DMA_destination(DMA_STRUCT *dma, void *p, unsigned int wordsize)
{ 	DMA_TCD *TCD=dma->TCD;

	if(!((wordsize==1)||(wordsize==2)||(wordsize==4))) return; // limit to 1,2,4
	TCD->DADDR = p;
	TCD->DOFF = 0;
	TCD->ATTR |= DMA_TCD_ATTR_DSIZE(wordsize/2);
	TCD->NBYTES = wordsize;
	TCD->DLAST_SGA = 0;
}

void DMA_destination_2ch(DMA_STRUCT *dma, void *p, unsigned int wordsize)
{ 	DMA_TCD *TCD=dma->TCD;

	if(!((wordsize==1)||(wordsize==2)||(wordsize==4))) return; // limit to 1,2,4
	TCD->DADDR = p;
	TCD->DOFF = 4; //always
//	TCD->ATTR |= (wordsize/2) <<8;
//	TCD->NBYTES = DMA_TCD_NBYTES_DMLOE |
//					DMA_TCD_NBYTES_MLOFFYES_MLOFF(-8) |
//					DMA_TCD_NBYTES_MLOFFYES_NBYTES(2*wordsize);
//	TCD->DLAST_SGA = -8;
	TCD->ATTR |= DMA_TCD_ATTR_DSIZE(wordsize/2) | DMA_TCD_ATTR_DMOD(3);
	TCD->NBYTES = 2*wordsize;
	TCD->DLAST_SGA = 0;
}

void DMA_destinationBuffer(DMA_STRUCT *dma, void *p, unsigned int len, unsigned int wordsize) 
{ 	DMA_TCD *TCD=dma->TCD;

	if(!((wordsize==1)||(wordsize==2)||(wordsize==4))) return; // limit to 1,2,4
	TCD->DADDR = p;
	TCD->DOFF = wordsize;
	TCD->ATTR |= DMA_TCD_ATTR_DSIZE(wordsize/2);
//	TCD->NBYTES = wordsize;
	TCD->DLAST_SGA = -wordsize*len; // is in bytes
	TCD->BITER = TCD->CITER = len; // number of major transfers
}

void DMA_destinationBuffer_2ch(DMA_STRUCT *dma, void *p, unsigned int len, unsigned int wordsize) 
{ 	DMA_TCD *TCD=dma->TCD;

	if(!((wordsize==1)||(wordsize==2)||(wordsize==4))) return; // limit to 1,2,4
	TCD->DADDR = p;
	TCD->DOFF = wordsize;
	TCD->ATTR |= DMA_TCD_ATTR_DSIZE(wordsize/2);
//	TCD->NBYTES = wordsize;
	TCD->DLAST_SGA = -wordsize*2*len; // is in bytes, is twice than size of single trandfer buffer
	TCD->BITER = TCD->CITER = len; // number of major transfers
}

void DMA_transferCount(DMA_STRUCT *dma, unsigned int len) 
{ 	DMA_TCD *TCD=dma->TCD;

	TCD->BITER = TCD->CITER = len; // number of words
}

void DMA_interruptAtCompletion(DMA_STRUCT *dma) 
{ 	DMA_TCD *TCD=dma->TCD;
	TCD->CSR |= DMA_TCD_CSR_INTMAJOR;
}

void DMA_interruptAtHalf(DMA_STRUCT *dma) 
{ 	DMA_TCD *TCD=dma->TCD;
	TCD->CSR |= DMA_TCD_CSR_INTHALF;
}

void DMA_start(DMA_STRUCT *dma) 
{ 	DMA_TCD *TCD=dma->TCD;
	TCD->CSR = DMA_TCD_CSR_START;
}

void DMA_enable(DMA_STRUCT *dma) 
{
	DMA_SERQ = dma->channel;
}

void DMA_disable(DMA_STRUCT *dma) 
{
	DMA_CERQ = dma->channel;
}

void DMA_disableOnCompletion(DMA_STRUCT *dma) 
{ 	DMA_TCD *TCD=dma->TCD;
	TCD->CSR |= DMA_TCD_CSR_DREQ;
}

void DMA_haltAll(void)
{ 	SET_BIT(DMA_CR,DMA_CR_HALT);
}

void DMA_startAll(void)
{ 	CLEAR_BIT(DMA_CR,DMA_CR_HALT);
}

bool DMA_complete(DMA_STRUCT *dma) 
{ 	DMA_TCD *TCD=dma->TCD;
	if (TCD->CSR & DMA_TCD_CSR_DONE) return true;
	return false;
}

void DMA_clearComplete(DMA_STRUCT *dma) 
{ 	DMA_CDNE = dma->channel;
}

bool DMA_error(DMA_STRUCT *dma) 
{	if (DMA_ERR & (1<<dma->channel)) return true;
	return false;
}

void DMA_clearError(DMA_STRUCT *dma) 
{	DMA_CERR = dma->channel;
}

uint32_t DMA_getError(void)
{ return DMA_ES;
}

void *DMA_sourceAddress(DMA_STRUCT *dma) 
{ 	DMA_TCD *TCD=dma->TCD;
	return (void *)(TCD->SADDR);
}

void *DMA_destinationAddress(DMA_STRUCT *dma) 
{ 	DMA_TCD *TCD=dma->TCD;
	return (void *)(TCD->DADDR);
}

//=====================================================================================	
void DMA_triggerAtHardwareEvent(DMA_STRUCT *dma, uint8_t source) 
{
	volatile uint8_t *mux;
	mux = (volatile uint8_t *)&(DMAMUX0_CHCFG0) + dma->channel;
	*mux = 0;
	*mux = (source & 63) | DMAMUX_ENABLE;
}

//=====================================================================================	
void DMA_attachInterrupt(DMA_STRUCT *dma, void (*isr)(void)) 
{
	_VectorsRam[dma->channel + IRQ_DMA_CH0 + 16] = isr;
	NVIC_ENABLE_IRQ(IRQ_DMA_CH0 + dma->channel);
}

void DMA_detachInterrupt(DMA_STRUCT *dma) 
{
	NVIC_DISABLE_IRQ(IRQ_DMA_CH0 + dma->channel);
}

void DMA_clearInterrupt(DMA_STRUCT *dma) 
{
	DMA_CINT = dma->channel;
}

/*****************************************************************************************/
DMA_STRUCT *cpy_dma =0;
//https://github.com/manitou48/teensy3/blob/master/mem2mem.pde
//https://forum.pjrc.com/threads/27752-teensy-3-memcpy-has-gotten-slower?p=64795&viewfull=1#post64795
void dmaCopy32(int *dest, int *src, unsigned int count) 
{ 
	volatile DMA_TCD *TCD;
	if(!cpy_dma) cpy_dma =DMA_allocate(0);
	//
	TCD=cpy_dma->TCD;
	
         TCD->SADDR = src; 
         TCD->SOFF = 4; 
         TCD->ATTR = DMA_TCD_ATTR_SSIZE(2) | DMA_TCD_ATTR_DSIZE(2); //32bit 
         TCD->NBYTES = count * 4; 
         TCD->SLAST = 0; 
         TCD->DADDR = dest; 
         TCD->DOFF = 4; 
         TCD->CITER = 1; 
         TCD->DLAST_SGA = 0; 
         TCD->BITER = 1; 
         TCD->CSR = DMA_TCD_CSR_START; 
		 // wait until finished
         while (!(TCD->CSR & DMA_TCD_CSR_DONE));
 } 
 
 
 void dmaSet32(int *dest, int val, unsigned int count) 
 { 
	DMA_STRUCT *dma =DMA_allocate(0);
	volatile DMA_TCD *TCD=dma->TCD;
	
         TCD->SADDR = &val; 
         TCD->SOFF = 0; 
         TCD->ATTR = DMA_TCD_ATTR_SSIZE(2) | DMA_TCD_ATTR_DSIZE(2); //32bit 
         TCD->NBYTES = count * 4; 
         TCD->SLAST = 0; 
         TCD->DADDR = dest; 
         TCD->DOFF = 4; 
         TCD->CITER = 1; 
         TCD->DLAST_SGA = 0; 
         TCD->BITER = 1; 
         TCD->CSR = DMA_TCD_CSR_START; 
		 // wait until finished
         while (!(TCD->CSR & DMA_TCD_CSR_DONE)); 
		 
	DMA_release(dma);
 } 


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
//dma.h

// WMXZ
//Inspired by DMAChannel.h
// converted to plain c
//
// example from http://cache.freescale.com/files/microcontrollers/doc/app_note/AN4765.pdf
// linked channel
/* Configure CH0 */
//DMA_0.TCD[0].SADDR = (int)&data_array0[0];
//DMA_0.TCD[0].SMOD = 0;
//DMA_0.TCD[0].SSIZE = 0x2; /* 32-bit */
//DMA_0.TCD[0].DMOD = 0;
//DMA_0.TCD[0].DSIZE = 0x2; /* 32-bit */
//DMA_0.TCD[0].SOFF = 0x4;
//DMA_0.TCD[0].NBYTES = 64; /* 16x32-bits */
//DMA_0.TCD[0].SLAST = -64;
//DMA_0.TCD[0].DADDR = 0x4001F000;
//DMA_0.TCD[0].CITER_ELINK = 0;
////DMA_0.TCD[0].CITER_LINKCH = 0;
//DMA_0.TCD[0].CITER = 1;
//DMA_0.TCD[0].DOFF = 0x4;
//DMA_0.TCD[0].DLAST_SGA = -64;
//DMA_0.TCD[0].BITER = 1;
//DMA_0.TCD[0].BITER_ELINK = 0;
////DMA_0.TCD[0].BITER_LINKCH = 0;
//DMA_0.TCD[0].BWC = 0;
//DMA_0.TCD[0].MAJORLINKCH = 1; /* Link to channel 1 */
//DMA_0.TCD[0].MAJORELINK = 1; /* Enable channel linking */
//DMA_0.TCD[0].ESG = 0;
//DMA_0.TCD[0].DREQ = 0;
//DMA_0.TCD[0].INTHALF = 0;
//DMA_0.TCD[0].INTMAJ = 0;

/* Configure CH1 */
//DMA_0.TCD[1].SADDR = (int)&data_array1[0];
//DMA_0.TCD[1].SMOD = 0;
//DMA_0.TCD[1].SSIZE = 0x2; /* 32-bit */
//DMA_0.TCD[1].DMOD = 0;
//DMA_0.TCD[1].DSIZE = 0x2; /* 32-bit */
//DMA_0.TCD[1].SOFF = 0x4;
//DMA_0.TCD[1].NBYTES = 16; /* 4x32-bits */
//DMA_0.TCD[1].SLAST = -16;
//DMA_0.TCD[1].DADDR = 0x4001FF00;
//DMA_0.TCD[1].CITER_ELINK = 0;
////DMA_0.TCD[1].CITER_LINKCH = 0;
//DMA_0.TCD[1].CITER = 1;
//DMA_0.TCD[1].DOFF = 0x4;
//DMA_0.TCD[1].DLAST_SGA = -16;
//DMA_0.TCD[1].BITER = 1;
//DMA_0.TCD[1].BWC = 0;
////DMA_0.TCD[1].MAJORLINKCH = 2; /* Link to channel 2 */
//DMA_0.TCD[1].MAJORELINK = 0; /* Enable channel linking */
//DMA_0.TCD[1].ESG = 0;
//DMA_0.TCD[1].DREQ = 0;
//DMA_0.TCD[1].INTHALF = 0;
//DMA_0.TCD[1].INTMAJ = 0;

//scatter gather
/* Configure CH0 */
//DMA_0.TCD[0].SADDR = (int)&data_array0[0];
//DMA_0.TCD[0].SMOD = 0;
//DMA_0.TCD[0].SSIZE = 0x2; /* 32-bit */
//DMA_0.TCD[0].DMOD = 0;
//DMA_0.TCD[0].DSIZE = 0x2; /* 32-bit */
//DMA_0.TCD[0].SOFF = 0x4;
//DMA_0.TCD[0].NBYTES = 56; /* 14x32-bits */
//DMA_0.TCD[0].SLAST = -56;
//DMA_0.TCD[0].DADDR = 0x4001F000;
//DMA_0.TCD[0].CITER_ELINK = 0;
////DMA_0.TCD[0].CITER_LINKCH = 0;
//DMA_0.TCD[0].CITER = 1;
//DMA_0.TCD[0].DOFF = 0x4;
//DMA_0.TCD[0].DLAST_SGA = (int)&TCD_SG[0];
//DMA_0.TCD[0].BITER = 1;
//DMA_0.TCD[0].BITER_ELINK = 0;
////DMA_0.TCD[0].BITER_LINKCH = 0;
//DMA_0.TCD[0].BWC = 0;
////DMA_0.TCD[0].MAJORLINKCH = 1; /* Link to channel 1 */
//DMA_0.TCD[0].MAJORELINK = 0; /* Disable channel linking */
//DMA_0.TCD[0].ESG = 1; /* Enable Sgatter/gather */
//DMA_0.TCD[0].DREQ = 0;
//DMA_0.TCD[0].INTHALF = 0;
//DMA_0.TCD[0].INTMAJ = 0;

//vuint32_t TCD_SG[] = {
//(int)&data_array1[0],0x02020004,
//0x00000008,0xfffffff8,
//0x4001ff00,0x00010004,
//0xfffffff8,0x00010001,
//};
#ifndef DMA_H
#define DMA_H
#include <stdbool.h>

#define DMAMEM __attribute__ ((section(".dmabuffers"), used))

typedef struct
{	void * SADDR;
	uint16_t SOFF;
	uint16_t ATTR;
	uint32_t NBYTES;
	uint32_t SLAST;
	void * DADDR;
	uint16_t DOFF;
	uint16_t CITER;
	uint32_t DLAST_SGA;
	uint16_t CSR;
	uint16_t BITER;
} DMA_TCD;


typedef struct
{	DMA_TCD *TCD;
	int channel;
	int prio;
} DMA_STRUCT;

#ifdef __cplusplus
extern "C"{
#endif

void DMA_init(void);
DMA_STRUCT *DMA_allocate(int prio) ;
void DMA_release(DMA_STRUCT *dma) ;
void DMA_source(DMA_STRUCT *dma, void *p, unsigned int wordsize) ;
void DMA_sourceBuffer(DMA_STRUCT *dma, void *p, unsigned int len, unsigned int wordsize) ;
void DMA_destination(DMA_STRUCT *dma, void *p, unsigned int wordsize) ;
void DMA_destinationBuffer(DMA_STRUCT *dma, void *p, unsigned int len, unsigned int wordsize) ;
void DMA_transferCount(DMA_STRUCT *dma, unsigned int len) ;
//=====================================================================================	
void DMA_source_2ch(DMA_STRUCT *dma, void *p, unsigned int wordsize) ;
void DMA_destinationBuffer_2ch(DMA_STRUCT *dma, void *p, unsigned int len, unsigned int wordsize) ;
void DMA_destination_2ch(DMA_STRUCT *dma, void *p, unsigned int wordsize) ;
void DMA_sourceBuffer_2ch(DMA_STRUCT *dma, void *p, unsigned int len, unsigned int wordsize) ;
//=====================================================================================	
void DMA_interruptAtCompletion(DMA_STRUCT *dma) ;
void DMA_interruptAtHalf(DMA_STRUCT *dma) ;
void DMA_disableOnCompletion(DMA_STRUCT *dma) ;
//=====================================================================================	
void DMA_enable(DMA_STRUCT *dma) ;
void DMA_disable(DMA_STRUCT *dma) ;
void DMA_start(DMA_STRUCT *dma) ;
void DMA_haltAll(void);
void DMA_startAll(void);
bool DMA_complete(DMA_STRUCT *dma) ;
void DMA_clearComplete(DMA_STRUCT *dma) ;
bool DMA_error(DMA_STRUCT *dma) ;
void DMA_clearError(DMA_STRUCT *dma) ;
uint32_t DMA_getError(void);
//
void *DMA_sourceAddress(DMA_STRUCT *dma) ;
void *DMA_destinationAddress(DMA_STRUCT *dma) ;
//=====================================================================================	
void DMA_triggerAtHardwareEvent(DMA_STRUCT *dma, uint8_t source) ;
//=====================================================================================	
void DMA_attachInterrupt(DMA_STRUCT *dma, void (*isr)(void)) ;
void DMA_detachInterrupt(DMA_STRUCT *dma) ;
void DMA_clearInterrupt(DMA_STRUCT *dma) ;
//=====================================================================================	
void dmaCopyInit(void);
void dmaCopy32(int *dest, int *src, unsigned int count);
void dmaSet32(int *dest, int val, unsigned int count);

#ifdef __cplusplus
}
/*
class dma_class
{	
private:
	DMA_STRUCT * dma;
public:
		dma_class dma_class(int prio) { dma = DMA_allocate(prio);}
		void ~dma_class() {DMA_release(dma);}
		//
		void source(uint32_t *p, unsigned int wordsize) 
				{DMA_source(dma, p, wordsize);}
		void source_2ch(uint32_t *p, unsigned int wordsize) 
				{DMA_source_2ch(dma, p, wordsize);}
		void sourceBuffer(void *p, unsigned int len, unsigned int wordsize) 
				{DMA_sourceBuffer(dma, p, len, wordsize);}
		void sourceBuffer_2ch(void *p, unsigned int len, unsigned int wordsize) 
				{DMA_sourceBuffer_2ch(dma, p, len, wordsize);}
		//
		void destination(uint32_t *p, unsigned int wordsize) 
				{DMA_destination(dma, p, wordsize);}
		void destination_2ch(uint32_t *p, unsigned int wordsize) 
				{DMA_destination_2ch(dma, p, wordsize);}
		void destinationBuffer(void *p, unsigned int len, unsigned int wordsize) 
				{DMA_destinationBuffer(dma, p, len, wordsize);}
		void destinationBuffer_2ch(void *p, unsigned int len, unsigned int wordsize) 
				{DMA_destinationBuffer_2ch(dma, p, len, wordsize);}
		//
		void transferCount(unsigned int len) 
				{DMA_transferCount(dma, len);}
		//=====================================================================================	
		void interruptAtCompletion(void)  
				{DMA_interruptAtCompletion(dma);}
		void interruptAtHalf(void) 
				{DMA_interruptAtHalf(dma);}
		void disableOnCompletion(void)  
				{DMA_disableOnCompletion(dma);}
		//=====================================================================================	
		void enable(void)  
				{DMA_enable(dma);}
		void disable(void)  
				{DMA_disable(dma);}
		bool complete(void)  
				{return DMA_complete(dma);}
		void clearComplete(void)  
				{DMA_clearComplete(dma);}
		bool error(void)  
				{return DMA_error(dma);}
		void clearError(void)  
				{DMA_clearError(dma);}
		void *sourceAddress(void)  
				{return DMA_sourceAddress(dma);}
		void *destinationAddress(void)  
				{return DMA_destinationAddress(dma);}
		//=====================================================================================	
		void triggerAtHardwareEvent(uint8_t source)  
				{DMA_triggerAtHardwareEvent(dma, source);}
		//=====================================================================================	
		void attachInterrupt(void (*isr)(void))  
				{DMA_attachInterrupt(dma, isr);}
		void detachInterrupt(void) 
				{DMA_detachInterrupt(dma);}
		void clearInterrupt(void)  
				{DMA_clearInterrupt(dma);}
};
*/
#endif

#endif
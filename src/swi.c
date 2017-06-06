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
// swi.c
// structure  take from xxxajk and with info from Frank B
// handles 5 swi slot simulaneously
// works only for Teeny 3.1(2)
//
#include "kinetis.h"

#include "jobs.h"
#include "swi.h"

typedef struct 
{	Task_Obj job;
	int irq;
	int count;
} SWI_OBJ;

#define INTERRUPT_OFF	{__disable_irq();}
#define INTERRUPT_ON	{__enable_irq();}

#define TRIGGER_SWI(irq, prio) {NVIC_CLEAR_PENDING(irq); NVIC_SET_PRIORITY(irq, prio); NVIC_TRIGGER_IRQ(irq);}

// unused IRQ Vectors mk20dx256
#ifdef __MK20DX256__
#define SWI_MAX_NUM 5
	int uswi[]={33,39,44, 53,54,55,56,57,58,59, 67,68,69,79,71,72, 91,92,93,94,95, 98, 102, 108,109}; // 
#endif
// unused IRQ Vectors mk66fx1m0
#ifdef __MK66FX1M0__
#define SWI_MAX_NUM 4
	int uswi[]={46,71,84,85}; //
#endif

SWI_OBJ swis[SWI_MAX_NUM];

// swi dispatcher
void swi_handle_ISR(int ii);
// ISR Handlers:
void unused_swi() {;}
void SWI_0(void) {	swi_handle_ISR(0);}
void SWI_1(void) {	swi_handle_ISR(1);}
void SWI_2(void) {	swi_handle_ISR(2);}
void SWI_3(void) {	swi_handle_ISR(3);}
void SWI_4(void) {	swi_handle_ISR(4);}

// list of Interrupt service routines <= SWI_MAX_NUM entries and to be defined below
void (* const SWIS[])(void) ={SWI_0, SWI_1, SWI_2, SWI_3, SWI_4};

void SWI_init(void)
{ int ii;
    for(ii=0;ii<SWI_MAX_NUM;ii++)
	{	_VectorsRam[uswi[ii]] = SWIS[ii];
		swis[ii].irq=uswi[ii]-16; 
		NVIC_ENABLE_IRQ(swis[ii].irq);
	}
}

int SWI_add(Fxn_t funct, Ptr state, Ptr data, int nice, int count) 
{       int rv = 0;
		for(rv=0;rv<SWI_MAX_NUM;rv++)
		{	SWI_OBJ *swi=&swis[rv];
			if(!swi->job.funct) // have empty slot
			{	swi->job.funct=funct;
				swi->job.state=state;
				swi->job.data=data;
				swi->job.nice=nice;
				swi->count=count;
				break;
			}
		}
        if(rv<SWI_MAX_NUM) return rv; else return -1; // -1 means too many active SWIs
}

int SWI_setBuffer(int rv, Ptr state, Ptr data)
{
	if(rv<0) return rv;
	{	SWI_OBJ *swi = &swis[rv];
		if(swi->job.funct)
		{
			swi->job.state=state;
			swi->job.data=data;
		}
		else
			rv=-1;
	}
	return rv;
}


// kinetis.h
// only T3.1(2)
// 0 = highest priority
// Cortex-M4: 0,16,32,48,64,80,96,112,128,144,160,176,192,208,224,240
int SWI_trigger(int rv, int prio) 
{
	if(rv<0) return rv;
    INTERRUPT_OFF
	{	SWI_OBJ *swi = &swis[rv];
		if(swi->job.funct)
			TRIGGER_SWI(swi->irq,prio*16) // do here the real triggering
		else
			rv=-1;
	}
	INTERRUPT_ON
	return rv;
}

//#define SWI_BLOCK_INTERRUPT
// generic SWI ISR handler
void swi_handle_ISR(int ii)
{
	SWI_OBJ *swi = &swis[ii];
#ifdef SWI_BLOCK_INTERRUPT
    INTERRUPT_OFF // switch off global interrupts
#endif
	if(swi->job.funct)
	{
		if(swi->job.nice<0)
			(swi->job.funct)(swi->job.state, swi->job.data);
		else
			JOB_add(swi->job.funct, swi->job.state, swi->job.data, swi->job.nice);
	}
    if(swi->count >0) swi->count--;
	if(!swi->count) swi->job.funct = 0; // mark as done
#ifdef SWI_BLOCK_INTERRUPT
    INTERRUPT_ON // enable global interrupts
#endif
}

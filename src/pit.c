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
 // pit.c
#include "kinetis.h"	// for timer
#include "core_pins.h"	// for millis()

#include "pit.h"

void pit_init(void)
{
	SIM_SCGC6 |= SIM_SCGC6_PIT;
	// turn on PIT     
	PIT_MCR = 0x00;
}

//============================== 100 ms counter ======================
#define DSECPERDAY (10*3600*24)
uint32_t timing[2]; // timer[0] deca_second; timer[1] days

void pit0_start(void) 
{
	// Timer 0     
	PIT_LDVAL0 = F_BUS/10; // setup timer 0 for (F_BUS/frequency) cycles     
	PIT_TCTRL0 = 2; // enable Timer 0 interrupts      
	PIT_TCTRL0 |= 1; // start Timer 0
	timing[0]=0;
	timing[1]=0;
//	NVIC_SET_PRIORITY(IRQ_PIT_CH0, 7*16);
	NVIC_ENABLE_IRQ(IRQ_PIT_CH0);
}
//
void pit0_isr(void)
{ //
	PIT_TFLG0=1;
	timing[0]++;
	if(timing[0]>=DSECPERDAY)
	{ timing[1]++; timing[0]-=DSECPERDAY;}
}

//==========================================TIMER ==========================================================
// used for timed execution // timer period in ms (typical 10 ms)
void pit1_start(uint32_t period)	
{ uint32_t cycles;
 cycles=(F_BUS/1000)*period-1;
 //
 // Timer 1     
 PIT_LDVAL1 = cycles; // setup timer 1 for (F_BUS/frequency) cycles     
 PIT_TCTRL1 = 2; // enable Timer 1 interrupts      
 PIT_TCTRL1 |= 1; // start Timer 1

//  NVIC_SET_PRIORITY(IRQ_PIT_CH1, 6*16);
  NVIC_ENABLE_IRQ(IRQ_PIT_CH1);
}

void timer_update(void);

FASTRUN void pit1_isr(void)
{	PIT_TFLG1=1;
	//__disable_irq();
	timer_update();
	//__enable_irq();
}

/******************************** timed execution *************************************************/
#include "jobs.h"
#include "list.h"

typedef	struct
{
	uint32_t	cnt ;	// how often (-1 is always)
	uint32_t	msec ;	// delay in ms (-1 is immediately)
	uint32_t	wait ;	// internal wait variable
	int		slot;	// slot where job is stored
} TIMER_WaitObj ;

#define NTIMER 20
static TIMER_WaitObj	m_timer[NTIMER] ;
static Node_t 			timerList[NTIMER]; // pre-allocated ojects
static List_t 			List2;
//
//-------------------------------------------------------------------------------
FASTRUN void timer_update(void)
{ 	//update timer objects
	uint32_t t0 = millis();
	
	int reschedule=0;
	Node_t *temp = List2.head;
    while(temp!=NULL)
    {   TIMER_WaitObj * wait = (TIMER_WaitObj *)(temp->meta);
		// check if time to activate
		if((t0 >= wait->wait))
		{	reschedule=0;
			if(temp->job.nice<0) // immediate execution
			{
				temp->job.funct(temp->job.state, temp->job.data);
			}
			else
      { if(getFxn(wait->slot)==temp->job.funct) // old job is still executing
        {  // delay to next interval
          reschedule=1;
        }
        else
  			{	// activate job
  				int ii=JOB_add(temp->job.funct, temp->job.state, temp->job.data, temp->job.nice);
          wait->slot=ii;
  			}
      }

			if(reschedule || (wait->cnt<0) || (wait->cnt>1))
      { // rescheduled, continuous or multiple runs: update wakeup time
				wait->wait = (t0 + wait->msec); 
      }
			else 
			{
				list_remove(&List2, temp->job.funct);
			}
      // descres run counter if multiple nun
			if((!reschedule) && wait->cnt) wait->cnt--;
		}
		temp=temp->next;
    }
}

//-------------------------------public functions ------------------------------------------------------------

void TIMER_setup(void)
{	list_init(&List2, timerList, NTIMER);
}

int TIMER_add(Fxn_t funct, Ptr state, Ptr data, int nice, int msec, int cnt)
{	int ii = list_insert(&List2, funct, state,data, nice);
	//
	m_timer[ii].cnt=cnt;
	m_timer[ii].msec=msec;
	m_timer[ii].wait=(millis() + msec);
  m_timer[ii].slot=ii;
	//
	List2.nodeList[ii].meta=(Ptr) &m_timer[ii];

    return ii;
}

int TIMER_cancel(Fxn_t funct)
{	return list_remove(&List2, funct);
}

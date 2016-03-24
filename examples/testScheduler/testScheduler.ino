// test program for WMXZ simple async scheduler
//
#include "jobs.h" // for JOB_* functions
#include "pit.h"  // for TIMER_* functions
#include "pdb.h"  // for DELAY_add
#include "swi.h"  // for SWI_* functions
/*
 *  given a user defined function:
 *    void funct(Ptr state,Ptr data) {}
 *  
 *  the following interfaces:
 *    int JOB_add(Fxn_t funct, Ptr state, Ptr data, int nice);
 *    int TIMER_add(Fxn_t funct, Ptr state, Ptr data, int nice, int msec, int count);
 *    void DELAY_add(Fxn_t funct, Ptr state, Ptr data, int nice, uint32_t uDelay);
 *    void SWI_add(Fxn_t funct, Ptr state, Ptr data, int nice, int count);
 *  are available for asynchronous execution
 *  
 *    int JOB_cancel(Fxn_t funct);
 *    int TIMER_cancel(Fxn_t funct);
 *  are available for removing functions from the (timed) activation list
 *  
 *  and
 *    int SWI_trigger(int rv, int prio) ;
 *  is available for trigger from software a functio directly or indirectly via JOB activation
 * 
 *  The parameters "Ptr state" and "Ptr data" are passed on to function without modification
 *  
 *  The parameter nice describes (sensu UNIX) the scheduling priority 
 *  nice=0 highest priority, nice=255 lowest priority
 *  special case:
 *    nice= -1
 *  indicates immediate execution (possibly after timer period or delay)
 *  
 *  for TIMER_add, DELAY_add and SWI_add
 *  a value nice>=0 uses the JOB scheduler to activate the function
 *  
 *  msec is period in milli seconds
 *  count is number of activations (count>0)
 *    count = -1 indicates continuous repetitions
 *    
 *  If after msec delay the function activation is still pending in the JOB scheduler, 
 *  the function is rescheduled at a later time (additional msec delay)
 *  
 *  uDelay is activation delay in micro seconds
 *
 *  the count variable in SWI_add limits the number of activations to count. 
 *  A count value of -1 leads to continuos activations
 *
 */

 /**********************************************************************/
void function1(Ptr s, Ptr d)
{   Serial.println("function1");
}

void testFunction(Ptr s, Ptr d)
{ volatile static int count=0;  
  int *rv =(int*) s;
  Serial.printf("testFunction %d %d\n\r",*rv,count++);
  
  // when necessary trigger software slot passed on by *rv = *((int*) s)
  // use NVIC priority 7*16
  if(!(count%5)) SWI_trigger(*rv, 7*16) ;
}

void blinkOFF(Ptr s, Ptr d)
{ digitalWriteFast(13,LOW);
}

//#define USE_TIMER
void blinkON(Ptr s, Ptr d)
{ 
  digitalWriteFast(13,HIGH);

  #ifdef USE_TIMER
    uint32_t msec=100;
    // add 'blinkOFF' to timed execution list
    TIMER_add((Fxn_t) blinkOFF, (Ptr) 0, (Ptr) 0, 1, msec, 1); // delay in milliseconds
  #else
    uint32_t usec=100000;
    // add 'blinkOFF' to delayed execution
    DELAY_add((Fxn_t) blinkOFF, (Ptr) 0, (Ptr) 0, 1, usec);   // delay in microseconds
  #endif
}

/**********************************************************************/
void setup() {
  // put your setup code here, to run once:
  //
  //while(!Serial);
  
  pinMode(13, OUTPUT);
  
  // initialize JOB scheduler
  JOB_init();

  // initialize periodic interval timer
  // used for periodic execution of functions
  pit_init();
  pit1_start(10); // timer for timer update function (here every 10 ms)

  // setup timed function handler
  TIMER_setup();

  // add 'testFunction' to timed execution list
  volatile static int rv=-1;
  TIMER_add((Fxn_t) testFunction, (Ptr) &rv, (Ptr) 0, 0, 5000, -1);

  // add 'blinkON' to timed execution list
  TIMER_add((Fxn_t) blinkON, (Ptr) 0, (Ptr) 0, 1, 500, -1);

  // initialize software trigger
  SWI_init();
  // add 'function1' to be triggered sometimes by software
  rv=SWI_add((Fxn_t) function1, (Ptr) 0, (Ptr) 0, 2, -1);
  
  Serial.printf("swi slot = %d\n\r",rv);
}

void loop() {
  // put your main code here, to run repeatedly:
  //
  // whenever possible we run JOB_scheduler
  static int old;
  int nc = JOB_schedule();
  if(nc != old) Serial.printf("number of active tasks: %d\n\r",nc);
  old=nc;
}

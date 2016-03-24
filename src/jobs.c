/*
 * WMXZ Teensy async scheduler
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
 // jobs.c
//
#include "jobs.h"
#include "list.h"

#define NUM_LIST 20
static Node_t nodeList[NUM_LIST]; // pre-allocated ojects
static List_t List1;
volatile static int job_busy=0;
//-------------------------------public functions ------------------------------------------------------------

void JOB_init(void)
{	list_init(&List1, nodeList, NUM_LIST);
}

int JOB_add(Fxn_t funct, Ptr state, Ptr data, int nice)
{	int ret;
  if(nice<0) 
  { funct(state,data); 
    return -1;
  } 
	job_busy=1;
	ret=list_insert(&List1, funct, state,data, nice);
	job_busy=0;
	return ret;
}

int JOB_cancel(Fxn_t funct)
{	return list_remove(&List1, funct);
}

FASTRUN int JOB_schedule(void)
{	
    Node_t *temp = List1.head;
    
	if(job_busy) return -1;
	
	if(temp)
	{	(temp->job.funct)(temp->job.state,temp->job.data);
		list_remove(&List1, temp->job.funct);
	}
	return node_count(&List1);
}

Fxn_t getFxn(int ii)
{ if(!List1.head) return 0;
  if(ii>=node_count(&List1)) return 0;
  return (Fxn_t) List1.nodeList[ii].job.funct;
}

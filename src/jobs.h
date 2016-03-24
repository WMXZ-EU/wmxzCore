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
 //jobs.h
 #ifndef JOBS_H
#define JOBS_H

#include "basics.h"

#define MNICE 255;

typedef struct  
{	Fxn_t	funct ;			// function to run when the job is scheduled
	Ptr		state ;			// private information for the job
	Ptr		data ;			// pointer to data to pass to the job when it runs
	int 	nice;       // store for nice value (-1 is immediate execution)
} Task_Obj ;

#ifdef __cplusplus
extern "C"{
#endif

void JOB_init(void);
int JOB_add(Fxn_t funct, Ptr state, Ptr data, int nice);
int JOB_schedule(void);
int JOB_cancel(Fxn_t funct);

Fxn_t getFxn(int n);

#ifdef __cplusplus
}
#endif

#endif

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
//compress.h

#ifndef COMPRESS_H
#define COMPRESS_H

#ifdef __cplusplus
extern "C"{
#endif

//#undef x3SHORT
//#define x3SHORT
#ifdef x3SHORT 	
	typedef short x3data_t;
	typedef unsigned short u3data_t; 
	typedef unsigned int l3data_t; 
	#define NBITS 16
#else
	typedef int x3data_t;
	typedef unsigned int u3data_t; 
	typedef unsigned long long l3data_t; 
	#define NBITS 32
#endif	


void u3_printv(u3data_t xx, int nn);
void x3_print(x3data_t xx);
void u3_print(u3data_t xx);
void l3_print(l3data_t xx);

int encode(u3data_t *out, x3data_t *inp, int nn, int ich, int nch);
void decode(x3data_t *out, u3data_t *inp, int nn, int nb, int ich, int nch);

int pack(u3data_t *out, u3data_t *inp, int nn, int nb);
int unpack(u3data_t *out, u3data_t *inp, int nn, int *nb, int km);

#ifdef __cplusplus
}
#endif

#endif
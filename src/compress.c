/*
 * WMXZ Teensy simple data logger
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
//compress.c

#include "compress.h"

#ifdef xxxx
void u3_printv(u3data_t xx, int nn)
{	int jj; for(jj=nn-1;jj>=0;jj--) printf("%d",(xx & (u3data_t)(1<<jj))>>jj);
}
void x3_print(x3data_t xx)
{	int jj; for(jj=8*sizeof(x3data_t)-1;jj>=0;jj--) printf("%d",(xx & (x3data_t)(1<<jj))>>jj);
}
void u3_print(u3data_t xx)
{	int jj; for(jj=8*sizeof(u3data_t)-1;jj>=0;jj--) printf("%d",(xx & (u3data_t)(1<<jj))>>jj);
}
void l3_print(l3data_t xx)
{	int jj; for(jj=8*sizeof(l3data_t)-1;jj>=0;jj--) printf("%d",(xx & (l3data_t)(1ll<<jj))>>jj);
}
#else
void u3_printv(u3data_t xx, int nn) {}
void x3_print(x3data_t xx) {}
void u3_print(u3data_t xx) {}
void l3_print(l3data_t xx) {}
#endif

int encode(u3data_t *out, x3data_t *inp, int nn, int ich, int nch)
{	int ii,nb;
	u3data_t ma=0;
	for(ii=0;ii<nn;ii++)
	{   x3data_t aa = inp[ich+ii*nch];
		if(aa<0) aa=-aa;
		if(ma<aa) ma=aa;
	}

	for(nb=0;ma>0; nb++,ma/=2); nb++;
    u3data_t msk = (1<<(nb+1))-1;
	
	for(ii=0;ii<nn;ii++)
	{ 	out[ii] = (u3data_t) inp[ich+ii*nch];
		out[ii] = out[ii] & msk;
	}
	return nb;
}

void decode(x3data_t *out, u3data_t *inp, int nn, int nb, int ich, int nch)
{	int ii;
	int shift=NBITS-(nb+1);
	for(ii=0;ii<nn;ii++)
	{	out[ich+ii*nch]=inp[ii];
		out[ich+ii*nch]=((x3data_t)(out[ich+ii*nch]<<shift))>>shift;
	}
}

int pack(u3data_t *out, u3data_t *inp, int nn, int nb)
{	int ii, kk;
	l3data_t temp,aux;
	int maxBits=8*sizeof(l3data_t);
	int lb=maxBits;
	temp=0; 

	kk=0;
	aux=nb;
	temp = aux << (lb-6); lb -= 7;

	for(ii=0;ii<nn;ii++)
	{
		aux=inp[ii];
		temp |= aux<<(lb-nb); lb -= nb+1;

		if(lb<NBITS)
		{	out[kk++]=(u3data_t) (temp >> NBITS); // copy top to output
			temp <<= NBITS;
			lb += NBITS;
		}
	}
	while(lb<maxBits)
	{	out[kk++]=(u3data_t) (temp >> NBITS); // copy top to output
		temp <<= NBITS;
		lb += NBITS;
	}
	return kk;
}	

int unpack(u3data_t *out, u3data_t *inp, int nn, int *nb, int km)
{	int ii,kk;
	l3data_t temp,aux;
	int maxBits=8*sizeof(l3data_t);
	int lb=maxBits;

	kk=0;
	temp=0;
	if(lb>NBITS && kk<km)
	{ 	aux=inp[kk++];
		temp |= aux<<(lb-NBITS); lb -= NBITS;
	}
	*nb = (int)(temp >> (maxBits-6));
	temp <<= 6; lb += 6;
	km = (6 + nn * (*nb+1) + 8*sizeof(u3data_t)-1)/(8*sizeof(u3data_t));
	//
	for(ii=0;ii<nn;ii++)
	{
		if(lb > NBITS && kk<km)
		{ 	aux=inp[kk++];
			temp |= aux<<(lb-NBITS); lb -= NBITS;
		}
		out[ii]=(u3data_t)(temp>>(maxBits-*nb-1));
		temp <<= *nb+1; lb += *nb+1;
	}
	return kk;
}

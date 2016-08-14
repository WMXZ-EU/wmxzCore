#include <stdio.h>
#include <stdlib.h>

#include "compress.h"

x3data_t rnd[256] = {
-111, -43, -56, 139, 100, 192, 455, 313,-329,-596, 231,-470,  17,   9, 570, -18,
-130,  60,  63,  18,-156,-313,  81,-344,-264, 341,-107, -36, 230, -77, 264, -88,
 259, 161, -55,-222,-267, -69,-112,-105, 252, -76, 293,-136, 249,-134,  45, 249,
-106,-112, 513, 243,-111, 166, -92, 181, 362,-411, 263, 373,  12, 447,  40,-317,
-562, -85, 183,  81, 106,-148,  37,-419,-195,-210, 133,  -4,-296,  -2,-177,-171,
 221,  29, 102, 226,  46, 141, 175, 300, 122, 362,   6, -12, 436,-130,  -1, 235,
  38, 360, 265,  75,-199, 145,-354,  63, 207,  55, 225, 522, 237,  68, 164, 109,
-337,-107, 314, -11, 149,-258,  17, 154,-349,  89, -47,-241, -10,-485,-545,-301,
-254,-300,-442,  74,-408,  28, 201,  -1,  24, -97,-380, -11, 246, 445,-110,-417,
  43,  96, -58,-294, 518,-604,-131,-338,-163,  81,  35,-182, 199, 159, 166,-109,
 268, 169, 642, 272, 296,  14,-330, -95,-194,-144, 142,-143,-229,-105, -41, 105,
-244,  81,  20, 339, -55, -34,-300,-355,  79, -64, 129,-229, 489,  31, 268, -58,
 -42, 177, 142,-287,-392,-281,-362,  15,-105, -94,-348, 200, 112, -23, 261,-224,
 106,  89,  89,-187,  84,-132,-229,-308, 266,-217, -44,-309, -76,-827,-278,-365,
-260, -55, -83, 498,-146, -64,-402,-122,-343,   8, 218, 103,-179,-417, 374, 525,
  31,-253, 307,-152,-120, 227,-355,-501, 108, 103,  24, 127, 277, 248,-146, 207};


#define NDAT (256)
#define NCH (4)
#define NCOMP (16)
  
x3data_t in1Data[NCH*NDAT];
u3data_t tmpData[NCH*NDAT];
u3data_t outData[NCH*NDAT];
x3data_t in2Data[NCH*NDAT];
x3data_t in3Data[NCH*NDAT];

//========================= Main =======================
int main(int argc, char *argv[])
{ 	int ii,jj,kk,km;
	int nb;
		
	for(ii=0; ii<NCH*NDAT;ii++) outData[ii]=0;
	for(ii=0; ii<NDAT; ii++) 
		for(jj=0;jj<NCH; jj++)
			in1Data[jj+ii*NCH]=rnd[(10*jj+ii)%256]/4;

	printf("%d %d %d\n",sizeof(x3data_t),sizeof(u3data_t),sizeof(l3data_t));

	jj=0;
	nb=encode(tmpData,in1Data,NCOMP,jj,NCH);
	decode(in2Data,tmpData,NCOMP,nb,jj,NCH);
	for(ii=0;ii<NCOMP;ii++)
		printf("%5d:%d %5d %5d\n",ii,jj,in1Data[jj+ii*NCH],in2Data[jj+ii*NCH]);

//
	u3data_t *outPtr = outData;
	for(ii=0;ii<NDAT;ii+=NCOMP)
	{	
		for(jj=0;jj<NCH;jj++)
		{	nb=encode(tmpData,&in1Data[ii*NCH],NCOMP,jj,NCH);
			km=pack(outPtr, tmpData,NCOMP,nb); outPtr += km;
		}
	}
	int maxkk=outPtr-outData;
	//
	outPtr = outData;
	for(ii=0;ii<NDAT;ii+=NCOMP)
	{ 
		for(jj=0;jj<NCH;jj++)
		{	int nrem=maxkk-(outPtr-outData);
			km=unpack(tmpData,outPtr,NCOMP,&nb,nrem); outPtr += km;
			decode(&in3Data[ii*NCH],tmpData,NCOMP,nb,jj,NCH);
		}
	}
	printf("\n");
//
	for(ii=0;ii<NDAT;ii++)
		for(jj=0;jj<NCH;jj++)
			printf("%5d:%d %5d %5d\n",ii,jj,in1Data[jj+ii*NCH],in3Data[jj+ii*NCH]);

	printf("%d %d %f\n", NDAT*NCH, maxkk, NDAT*NCH/(float)maxkk); 

	return 0;
}

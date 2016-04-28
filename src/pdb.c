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
 // pdb.c
//=========================== PDB=====================================
#include "kinetis.h"
//
#include "jobs.h"
#include "pdb.h"
//
#define PDB_CONFIG (PDB_SC_TRGSEL(15) | PDB_SC_PDBEN | PDB_SC_PDBIE)
#define fbus (F_BUS/1000000L)
//
Task_Obj m_pdb_job;

void pdb_isr(void)
{//	 __disable_irq();
	 
	PDB0_SC = 0;//PDB_CONFIG | PDB_SC_LDOK;
  //
	if(m_pdb_job.nice<0)
		m_pdb_job.funct(m_pdb_job.state, m_pdb_job.data);
	else
		JOB_add(m_pdb_job.funct, m_pdb_job.state, m_pdb_job.data, m_pdb_job.nice);

//	 __enable_irq();
}

void DELAY_add(Fxn_t funct, Ptr state, Ptr data, int nice, uint32_t uDelay)
{	int ii,jj;
	uint32_t mult=0;
	uint16_t idelay; 

	m_pdb_job.funct=funct; // store for delayed execution
	m_pdb_job.state=state; // store for context data
	m_pdb_job.data=data; // store for data
	m_pdb_job.nice=nice; // store fo nice value (-1 is immediate execution)
	
	if(uDelay>6990400) uDelay=6990400; // limit to max achievable delay 

	for(jj=0; jj<4; jj++)
	{ 	if(jj==0) mult=1; else mult=10*(1<<(jj-1));
		for (ii=0; ii<8; ii++)
		{	if((uDelay*fbus/(mult*(1L<<ii))) < (1L<<16)) break;
		}
		if(ii<8) break;
	}
	idelay=(uint16_t) (uDelay*fbus/(mult*(1<<ii)));
	
	SIM_SCGC6 |= SIM_SCGC6_PDB;
	PDB0_IDLY = idelay;
	PDB0_MOD = idelay;
	PDB0_SC = PDB_CONFIG |  PDB_SC_MULT(jj) | PDB_SC_PRESCALER(ii) ;
	PDB0_SC |= PDB_SC_LDOK ;
	PDB0_CH0C1 = 0x0000;

//	NVIC_SET_PRIORITY(IRQ_PDB, 7*16);
	NVIC_ENABLE_IRQ(IRQ_PDB);

	PDB0_SC |= PDB_SC_SWTRIG;
}

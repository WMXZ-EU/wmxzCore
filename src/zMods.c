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
//zMods.c

#include "kinetis.h"
#include "zMods.h"

void zMods(void)
{
	#ifdef xxxxxx
	// allow overclocking F_BUS
	#if F_CPU == 144000000
		// config divisors: 144 MHz core, 144,72,48,36 MHz bus, 28.8 MHz flash
		
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV4(4);
//		SIM_CLKDIV1 = | SIM_CLKDIV1_OUTDIV2(((F_CPU/F_BUS)-1)); // does it work reliable?
//
		#if F_BUS==(F_CPU)
			SIM_CLKDIV1 = | SIM_CLKDIV1_OUTDIV2(0); // 144 MHz
		#elif F_BUS==(F_CPU/2)
			SIM_CLKDIV1 = | SIM_CLKDIV1_OUTDIV2(1); // 72 MHz
		#elif F_BUS==(F_CPU/3)
			SIM_CLKDIV1 = | SIM_CLKDIV1_OUTDIV2(2); // 48 MHz
		#elif F_BUS==(F_CPU/4)
			SIM_CLKDIV1 = | SIM_CLKDIV1_OUTDIV2(3); // 36 MHz
		#endif
//		((F_CPU/F_BUS)-1)
	#endif
	#endif
}
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
//i2s.h
// 20-may-17: added ICS43432

#ifndef I2S_H
#define I2S_H

#define I2S_TX_2CH 1
#define I2S_RX_2CH 2

#define PJRC_AUDIO_DEV	0
#define SGTL5000_DEV	1
#define CS5361_DEV		2
#define AD7982_DEV		3
#define ADS8881_DEV		4
#define ICS43432_DEV	5

#define USE_I2S_PIN
#ifndef I2S_PIN
	#define I2S_PIN (6)
#endif

typedef struct {
	int nbytes;
	int nsamp;
	int nchan;
} i2s_context_t ;

#ifdef __cplusplus
extern "C"{
#endif

void i2s_init(void);
float i2s_speedConfig(int device, int nbits, int fs);
void i2s_config(int isMaster, int nbits, int dual, int sync);
void i2s_configurePorts(int iconf);
void i2s_setupOutput(void * buffer, int ndat, int port, int prio);
void i2s_startOutput(void);

void i2s_setupInput(void * buffer, int ndat, int port, int prio);
void i2s_startInput(void);
void i2s_stop(void);

void i2s_enableInputDMA(void);
void i2s_enableOutputDMA(void);

void i2sInProcessing(void * s, void * d);
void i2sOutProcessing(void * s, void * d);


#ifdef __cplusplus
}

class C_I2S
{
public:
	void init(void) {i2s_init();}
	float i2s_speedConfig(int device, int nbits, int fs) {i2s_speedConfig(device, nbits, fs);}
	void i2s_config(int isMaster, int nbits, int dual, int sync) {i2s_config(isMaster, nbits, dual, sync)};
	void i2s_configurePorts(int iconf) {i2s_configurePorts(iconf)} 

	void i2s_setupOutput(void * buffer, int ndat, int port, int prio) {	i2s_setupOutput(buffer, ndat, port, prio);} 
	void i2s_startOutput(void) { i2s_startOutput();}

	void i2s_setupInput(void * buffer, int ndat, int port, int prio) { i2s_setupInput(buffer, ndat, port, prio);}
	void i2s_startInput(void) { i2s_startInput();}
	void i2s_stop(void) { i2s_stop();}

	void i2s_enableInputDMA(void) { i2s_enableInputDMA(); }
	void i2s_enableOutputDMA(void) { i2s_enableOutputDMA(); }

};

C_I2S i2s;
#endif //_cplusplus

#endif //I2S_H

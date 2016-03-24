/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 // Simplified and adapter by WMXZ

#ifndef control_sgtl5000_h_
#define control_sgtl5000_h_

#define AUDIO_INPUT_LINEIN  0
#define AUDIO_INPUT_MIC     1


	bool SGTL5000_muted;
	uint16_t SGTL5000_ana_ctrl;
	bool SGTL5000_semi_automated;


	unsigned int _read(unsigned int reg);
	bool _write(unsigned int reg, unsigned int val);
	unsigned int _modify(unsigned int reg, unsigned int val, unsigned int iMask);
	
	
#ifdef __cplusplus
extern "C"{
#endif
	bool SGTL5000_enable(int device, int mode);
	bool SGTL5000_inputSelect(int device, int n);
	bool SGTL5000_lineInLevel(int device, uint8_t left, uint8_t right);
	
	bool SGTL5000_volumeInteger(int device, unsigned int n); // range: 0x00 to 0x80
#ifdef __cplusplus
}
#endif
	/*
	bool SGTL5000_disable(void);
	bool SGTL5000_volume(float n);
	bool SGTL5000_inputLevel(float n);
	bool SGTL5000_muteHeadphone(void);
	bool SGTL5000_unmuteHeadphone(void);
	bool SGTL5000_muteLineout(void);
	bool SGTL5000_unmuteLineout(void);
	
//	bool SGTL5000_volume(float left, float right);
	bool SGTL5000_micGain(unsigned int dB);
//	bool SGTL5000_lineInLevel(uint8_t n) { return SGTL5000_lineInLevel(n, n); }
	bool SGTL5000_lineInLevel(uint8_t left, uint8_t right);
//	unsigned short SGTL5000_lineOutLevel(uint8_t n);
	unsigned short SGTL5000_lineOutLevel(uint8_t left, uint8_t right);
	unsigned short SGTL5000_dacVolume(float n);
//	unsigned short SGTL5000_dacVolume(float left, float right);
	unsigned short SGTL5000_adcHighPassFilterEnable(void);
	unsigned short SGTL5000_adcHighPassFilterFreeze(void);
	unsigned short SGTL5000_adcHighPassFilterDisable(void);
	unsigned short SGTL5000_audioPreProcessorEnable(void);
	unsigned short SGTL5000_audioPostProcessorEnable(void);
	unsigned short SGTL5000_audioProcessorDisable(void);
	unsigned short SGTL5000_eqFilterCount(uint8_t n);
	unsigned short SGTL5000_eqSelect(uint8_t n);
	unsigned short SGTL5000_eqBand(uint8_t bandNum, float n);
	void SGTL5000_eqBands(float bass, float mid_bass, float midrange, float mid_treble, float treble);
//	void SGTL5000_eqBands(float bass, float treble);
	void SGTL5000_eqFilter(uint8_t filterNum, int *filterParameters);
	unsigned short aSGTL5000_utoVolumeControl(uint8_t maxGain, uint8_t lbiResponse, uint8_t hardLimit, float threshold, float attack, float decay);
	unsigned short SGTL5000_autoVolumeEnable(void);
	unsigned short SGTL5000_autoVolumeDisable(void);
//	unsigned short SGTL5000_enhanceBass(float lr_lev, float bass_lev);
	unsigned short SGTL5000_enhanceBass(float lr_lev, float bass_lev, uint8_t hpf_bypass, uint8_t cutoff);
	unsigned short SGTL5000_enhanceBassEnable(void);
	unsigned short SGTL5000_enhanceBassDisable(void);
//	unsigned short SGTL5000_surroundSound(uint8_t width);
	unsigned short SGTL5000_surroundSound(uint8_t width, uint8_t select);
	unsigned short SGTL5000_surroundSoundEnable(void);
	unsigned short SGTL5000_surroundSoundDisable(void);
	void SGTL5000_killAutomation(void);

	unsigned char SGTL5000_calcVol(float n, unsigned char range);
	
	unsigned short SGTL5000_dap_audio_eq_band(uint8_t bandNum, float n);

//	void SGTL5000_automate(uint8_t dap, uint8_t eq);
	void SGTL5000_automate(uint8_t dap, uint8_t eq, uint8_t filterCount);
//For Filter Type: 0 = LPF, 1 = HPF, 2 = BPF, 3 = NOTCH, 4 = PeakingEQ, 5 = LowShelf, 6 = HighShelf
  #define FILTER_LOPASS 0
  #define FILTER_HIPASS 1
  #define FILTER_BANDPASS 2
  #define FILTER_NOTCH 3
  #define FILTER_PARAEQ 4
  #define FILTER_LOSHELF 5
  #define FILTER_HISHELF 6
  
//For frequency adjustment
  #define FLAT_FREQUENCY 0
  #define PARAMETRIC_EQUALIZER 1
  #define TONE_CONTROLS 2
  #define GRAPHIC_EQUALIZER 3


void calcBiquad(uint8_t filtertype, float fC, float dB_Gain, float Q, uint32_t quantization_unit, uint32_t fS, int *coef);
*/

#endif

/*------------------------------------------------------------------------------------------------*/
/* ___::((xodCpuCtrl.n))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((ODMK:2017:2022))::___
   ___::((created by eschei))___

	Purpose: XODMK EFX - CPU Control Functions (Embedded ARM)
	Device: All
	Revision History: 2023_09_23 - initial
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/


#ifndef __XODCPUCTRL_H__
#define __XODCPUCTRL_H__


#include <math.h>
#include <utility>
#include <cassert>
#include <ap_int.h>
#include <ap_fixed.h>

#include "xodEFXII_types.h"
#include "xodEFXII.h"


using namespace std;


// *--------------------------------------------------------------------------------* //
// *** CPU CTRL Functions ***

static float rmsValue(int arr[], int arrSize);

void setFDNChanDly(int maxChannel, float ddlChannelBase, float fdnLenArray[]);

template <class T, int channels>
void ddlxnLengthCtrl(bool centerLock, double delayBase,
		             double tilt, T ddlLengthArray[channels]);

//template <int channels>
//void ddlxnLengthCtrlAXI(bool centerLock, double delayBase,
//		         	    double tilt, uint32_t ddlLengthCtrlAgg[3]);

template <int channels>
void ddlxnLengthCtrlSW(bool centerLock, double delayBase,
		                double tilt, double ddlLengthArray[channels]);

void ddlxnLengthAXIBus(const double ddlLengthArray[4], uint32_t ddlLengthCtrlAgg[3]);


template <class T, int channels, int phaseWidth>
void ssbModFreqCtrl(bool centerLock, double freqBase,
		    		double tilt, T phaseStepArray[channels]);

template <int channels, int phaseWidth>
void ssbModFreqCtrlAXI(bool centerLock, double freqBase,
		       	   	   double tilt, uint32_t phaseStepAgg[channels]);


float onePoleTPT_G(float sr, float cutoff);

void setFcAndRes_MHL(float sr, float cutoff, float resonance, fParamU_t& fParamSW);

void setFcAndRes_ML4P(float sr, float cutoff, float resonance, fParamU_t& fParamSW);
void setFParamHW_ML4P(fParamU_t fParam_SW, fltCtrl_t& K_HW, fltCtrl_t& G_HW, fParam_t& fParam_HW);

float xodSEMSVF_setQ_ref(float Q);
void xodSEMSVF_update(float sr, float cutoff, float Q, float& alpha0, float& alpha, float& rho);


lfoFreq_t lfoFreq2phaseStep(float sr, double outputFreq);
uint16_t lfoFreq2phaseStep16b(float sr, double outputFreq);


// *--------------------------------------------------------------------------------* //
// *** Util Fader Curve Functions ***

inline float AmplitudeTodB(float amplitude)
{
	return 20.0f * log10(amplitude);
}

inline float dBToAmplitude(float dB)
{
	return pow(10.0f, dB / 20.0f);
}


// ** exponential Fader curve functions
// input linear value x in range [0:1] -> outputs exponential value x in range [0:1]
const int expCurveOrder = 2;       // factor to increase amount of exponential curve
const float scaleFactorExpSqrX = 1 / 1.718281828459045;

// inline float expSqrNorm(float x);
// inline float invExpSqrNorm(float x);

inline float expSqrNorm(float x)
{
    assert(0.0 <= x <= 1.0);
    float expSqrX = pow(x, expCurveOrder);
    // normalize data: zi = (xi - min(x)) / (max(x) - min(x))
    // normExpSqrX = (expSqrX - np.exp(0)) / (np.exp(1) - np.exp(0))
    float normExpSqrX = (expSqrX - 1.0) * scaleFactorExpSqrX;
    return normExpSqrX;
}

// ** Good for Log-Like Faders more sensitive @ high levels
// Usage:
// float feedbackMix			= 0.93;
// float expFLoat_fbMix 		= invExpSqrNorm(feedbackMix);
// delayCtrl_t expFade_fbMix 	= static_cast<delayCtrl_t>(expFLoat_fbMix);

inline float invExpSqrNorm(float x)
{
    assert(0.0 <= x <= 1.0);
    float expSqrX = pow((1.0 - x), expCurveOrder);
    // normalize data: zi = (xi - min(x)) / (max(x) - min(x))
    // normExpSqrX = (expSqrX - np.exp(0)) / (np.exp(1) - np.exp(0))
    float normExpSqrX = (expSqrX - 1.0) * scaleFactorExpSqrX;
    float invNormX = 1.0 - normExpSqrX;
    return invNormX;
}


// *--------------------------------------------------------------------------------* //
// __ddlxnLengthCtrl__
//
// USEAGE:
// ddlxnLengthCtrl<delayLen_t, channels>(ap_uint<1> centerLock, double delayBase, double tilt, T ddlLengthArray[channels]);
// ddlxnLengthCtrl<delayLen_t, 4>(1, 566.6, 56, ddlLengthArray);

template <class T, int channels>
void ddlxnLengthCtrl(bool centerLock, double delayBase,
		             double tilt, T ddlLengthArray[channels])
{
	// centerLock: locks two center step values to base frequency (else both sides shift from base freq!)
	assert(channels % 2 == 0);
	assert(channels <= 8);

	if (centerLock) {
		for (int c = 0; c < channels; c++) {
			if ((c == channels / 2 - 1) || (c == channels / 2)) {
				ddlLengthArray[c] = delayBase;
			} else if (c < channels / 2 - 1) {
				ddlLengthArray[c] = delayBase - (( (channels / 2 - 1) - c) * tilt);
			} else {
				ddlLengthArray[c] = delayBase + ((c - channels/2) * tilt);
			}
		}

	} else {
		for (int c = 0; c < channels; c++) {
			if (c < channels / 2) {
				ddlLengthArray[c] = delayBase - ((channels/2 - c) * tilt);
			} else {
				ddlLengthArray[c] = delayBase + ((c - channels/2 + 1) * tilt);
			}
		}

	}
}


// testbench function to mimic ARM software CTRL function
template <int channels>
void ddlxnLengthCtrlSW(bool centerLock, double delayBase,
		                double tilt, double ddlLengthArray[channels])
{
	// centerLock: locks two center step values to base frequency (else both sides shift from base freq!)
	assert(channels % 2 == 0);

	if (centerLock) {
		for (int c = 0; c < channels; c++) {
			if ((c == channels / 2 - 1) || (c == channels / 2)) {
				ddlLengthArray[c] = delayBase;
			} else if (c < channels / 2 - 1) {
				ddlLengthArray[c] = delayBase - (( (channels / 2 - 1) - c) * tilt);
			} else {
				ddlLengthArray[c] = delayBase + ((c - channels/2) * tilt);
			}
		}

	} else {
		for (int c = 0; c < channels; c++) {
			if (c < channels / 2) {
				ddlLengthArray[c] = delayBase - ((channels/2 - c) * tilt);
			} else {
				ddlLengthArray[c] = delayBase + ((c - channels/2 + 1) * tilt);
			}
		}
	}

}


// *--------------------------------------------------------------------------------* //
// __ssbModFreqCtrl__
//
// USEAGE:
// ssbModFreqCtrl<ddsCtrl_t, channels, phaseWidth>
//					(ap_uint<1> centerLock, double freqBase, double tilt, T phaseStepArray[channels]);
// ssbModFreqCtrl<ddsCtrl_t, 4, phaseWidth>(1, 566.6, 56, phaseStepArr);
//
// +/-1499 Hz max/min freq ssbOsc CTRL UI (ddsCtrl_t = 32bits)
// (-1499 * (2**36 / 48000) = -2146051992  =>  2**31 - 1 = 2147483647 (1500 overflows 32 bits)

template <class T, int channels, int phaseWidth>
void ssbModFreqCtrl(bool centerLock, double freqBase,
		            double tilt, T phaseStepArray[channels])
{
	// centerLock: locks two center step values to base frequency (else both sides shift from base freq!)
	assert(channels % 2 == 0);
	assert(channels <= 8);
	double ddsOutFreqInit[channels] {0};

	if (centerLock) {
		for (int c = 0; c < channels; c++) {
			if ((c == channels / 2 - 1) || (c == channels / 2)) {
				ddsOutFreqInit[c] = freqBase;
			} else if (c < channels / 2 - 1) {
				ddsOutFreqInit[c] = freqBase - (( (channels / 2 - 1) - c) * tilt);
			} else {
				ddsOutFreqInit[c] = freqBase + ((c - channels/2) * tilt);
			}
		}

	} else {
		for (int c = 0; c < channels; c++) {
			if (c < channels / 2) {
				ddsOutFreqInit[c] = freqBase - ((channels/2 - c) * tilt);
			} else {
				ddsOutFreqInit[c] = freqBase + ((c - channels/2 + 1) * tilt);
			}
		}

	}

	for (int c = 0; c < channels; c++) {
		//phaseStepAgg[c] = static_cast<uint32_t>(ddsOutFreqInit[c] * pow(2, ddsPhaseWidth) / sr);
		double phaseStepRaw = ddsOutFreqInit[c] * pow(2, ddsPhaseWidth) / sr;
		phaseStepArray[c] = static_cast<uint32_t>(phaseStepRaw / ddsPhaseScale);

		std::cout << "xodEFX_tb ssbModFreqCtrl CH[" << c << "]: ddsOutFreq = " << ddsOutFreqInit[c]
			 << ",    ddsStepRaw = " << phaseStepRaw
			 << ",    ddsStepVal = " << phaseStepArray[c] << std::endl;
	}
}


template <int channels, int phaseWidth>
void ssbModFreqCtrlAXI(bool centerLock, double freqBase,
		               double tilt, uint32_t phaseStepArray[channels])
{
	// centerLock: locks two center step values to base frequency (else both sides shift from base freq!)
	assert(channels % 2 == 0);
	assert(channels <= 8);
	double ddsOutFreqInit[channels];

	if (centerLock) {
		for (int c = 0; c < channels; c++) {
			if ((c == channels / 2 - 1) || (c == channels / 2)) {
				ddsOutFreqInit[c] = freqBase;
			} else if (c < channels / 2 - 1) {
				ddsOutFreqInit[c] = freqBase - (( (channels / 2 - 1) - c) * tilt);
			} else {
				ddsOutFreqInit[c] = freqBase + ((c - channels/2) * tilt);
			}
		}

	} else {
		for (int c = 0; c < channels; c++) {
			if (c < channels / 2) {
				ddsOutFreqInit[c] = freqBase - ((channels/2 - c) * tilt);
			} else {
				ddsOutFreqInit[c] = freqBase + ((c - channels/2 + 1) * tilt);
			}
		}

	}

	for (int c = 0; c < channels; c++) {
		//phaseStepAgg[c] = static_cast<uint32_t>(ddsOutFreqInit[c] * pow(2, ddsPhaseWidth) / sr);
		double phaseStepRaw = ddsOutFreqInit[c] * pow(2, ddsPhaseWidth) / sr;
		phaseStepArray[c] = static_cast<uint32_t>(phaseStepRaw / ddsPhaseScale);

		std::cout << "xodEFX_tb - ssbModFreqCtrlAXI CH[" << c << "]: ddsOutFreq = " << ddsOutFreqInit[c]
			 << ",    ddsStepRaw = " << phaseStepRaw
			 << ",    ddsStepVal = " << phaseStepArray[c] << std::endl;
	}
}


// *--------------------------------------------------------------------------------* //

#endif

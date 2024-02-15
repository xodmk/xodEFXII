/*------------------------------------------------------------------------------------------------*/
/* ___::((xodCpuCtrl.cpp))::___

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

#include <math.h>

#include "xodEFXII_types.h"
#include "xodEFXII.h"
#include "xodCpuCtrl.h"


// *--------------------------------------------------------------------------------* //
// *** Util Functions ***


// Function that Calculate Root Mean Square
// -> int arrSize = sizeof(arr)/sizeof(arr[0]);
static float rmsValue(int arr[], int arrSize)
{
    int square = 0;
    float mean, rootValue = 0.0;

    // Calculate square value.
    for (int i = 0; i < arrSize; i++) {
        square += pow(arr[i], 2);
    }

    // Calculate Mean value.
    mean = (square / static_cast<float>(arrSize));

    // Calculate Root value.
    rootValue = sqrt(mean);

    return rootValue;
}


// *--------------------------------------------------------------------------------* //
// *** Delay CTRL Functions ***


// distribute delay lengths evenly across channels
void setFDNChanDly(int maxChannel, float ddlChannelBase, float fdnLenArray[])
{
	const float maxChannelInv = 1.0 / MAXCHANNELS;

	for (int i = 0; i < maxChannel; i++) {

		// Distribute delay times exponentially between delayMs and 2*delayMs
		float r = i * maxChannelInv;
		fdnLenArray[i] = pow(2, r) * ddlChannelBase;
	}
}


//// *--------------------------------------------------------------------------------* //
//// __ddlxnLengthCtrl__
////
//// USEAGE:
//// ddlxnLengthCtrl<delayLen_t, channels>(ap_uint<1> centerLock, double delayBase, double tilt, T ddlLengthArray[channels]);
//// ddlxnLengthCtrl<delayLen_t, 4>(1, 566.6, 56, ddlLengthArray);
//
//template <class T, int channels>
//void ddlxnLengthCtrl(bool centerLock, double delayBase,
//		             double tilt, T ddlLengthArray[channels])
//{
//	// centerLock: locks two center step values to base frequency (else both sides shift from base freq!)
//	assert(channels % 2 == 0);
//	assert(channels <= 8);
//
//	if (centerLock) {
//		for (int c = 0; c < channels; c++) {
//			if ((c == channels / 2 - 1) || (c == channels / 2)) {
//				ddlLengthArray[c] = delayBase;
//			} else if (c < channels / 2 - 1) {
//				ddlLengthArray[c] = delayBase - (( (channels / 2 - 1) - c) * tilt);
//			} else {
//				ddlLengthArray[c] = delayBase + ((c - channels/2) * tilt);
//			}
//		}
//
//	} else {
//		for (int c = 0; c < channels; c++) {
//			if (c < channels / 2) {
//				ddlLengthArray[c] = delayBase - ((channels/2 - c) * tilt);
//			} else {
//				ddlLengthArray[c] = delayBase + ((c - channels/2 + 1) * tilt);
//			}
//		}
//
//	}
//}
//
//
//// testbench function to mimic ARM software CTRL function
//template <int channels>
//void ddlxnLengthCtrlSW(bool centerLock, double delayBase,
//		                double tilt, double ddlLengthArray[channels])
//{
//	// centerLock: locks two center step values to base frequency (else both sides shift from base freq!)
//	assert(channels % 2 == 0);
//
//	if (centerLock) {
//		for (int c = 0; c < channels; c++) {
//			if ((c == channels / 2 - 1) || (c == channels / 2)) {
//				ddlLengthArray[c] = delayBase;
//			} else if (c < channels / 2 - 1) {
//				ddlLengthArray[c] = delayBase - (( (channels / 2 - 1) - c) * tilt);
//			} else {
//				ddlLengthArray[c] = delayBase + ((c - channels/2) * tilt);
//			}
//		}
//
//	} else {
//		for (int c = 0; c < channels; c++) {
//			if (c < channels / 2) {
//				ddlLengthArray[c] = delayBase - ((channels/2 - c) * tilt);
//			} else {
//				ddlLengthArray[c] = delayBase + ((c - channels/2 + 1) * tilt);
//			}
//		}
//	}
//
//}


// TEMP - fixed 4-channel AXI bus conversion
void ddlxnLengthAXIBus(const double ddlLengthArray[4], uint32_t ddlLengthAXIBus[3])
{

	// TEMP DEBUG
	//	for (int c = 0; c < static_cast<int>(MAXCHANNELS); c++) {
	//		cout << "xodEFX_test[" << c << "]: ddlLengthArray = " << static_cast<float>(ddlLengthArray[c])
	//			 << ",    ddlLengthArray uint32 = " << static_cast<uint32_t>(ddlLengthArray[c])
	//			 << endl;
	//	}

	// 2**16 = 65536; 2**24 = 16777216; 2**32 = 4294967296
	// Agg[0] = ddlLen1 8lsb & ddlLen0 24all
	ddlLengthAXIBus[0] = static_cast<uint32_t>( (static_cast<uint32_t>((ddlLengthArray[1] * 4294967296)) & 4278190080)	// 4278190080 = 11111111000000000000000000000000
											   | (static_cast<uint32_t>((ddlLengthArray[0] * 256)) & 16777215) );		// 16777215   = 00000000111111111111111111111111
	// Agg[1] = ddlLen2 16lsb & ddlLen1 16msb
	ddlLengthAXIBus[1] = static_cast<uint32_t>( (static_cast<uint32_t>((ddlLengthArray[2] * 16777216)) & 4294901760)   // 4294901760 = 11111111111111110000000000000000
											   | (static_cast<uint32_t>(ddlLengthArray[1]) & 65535)	);					// 65535 = 1111111111111111
	// Agg[2] = ddlLen3 24all & ddlLen2 8msb
	ddlLengthAXIBus[2] = static_cast<uint32_t>( (static_cast<uint32_t>(ddlLengthArray[3] * 65536) & 4294967040)	// 4294967040 = 11111111111111111111111100000000
											   | ((static_cast<uint32_t>(ddlLengthArray[2]) >> 8) & 255) );

}


// *--------------------------------------------------------------------------------* //
// *** FIlter CTRL Functions ***

// *---------------------------------------------------------------------------* //
///// Calculate prewarp & 'big G' (Zavalishin p46) /////////////////////

float onePoleTPT_G(float sr, float cutoff)
{
	float G = 0;

	// prewarp the cutoff - bilinear-transform filters
	float wd = 2 * pi * cutoff;
	float T = 1 / sr;
	float wa = (2/T) * tan(wd * T/2);
	float g = wa * T/2;

	G = g / (1.0 + g);

	return G;
}


// *-----------------------------------------------------------------------------------* //
///// ARM software function /////////////////////

void setFcAndRes_MHL(float sr, float cutoff, float resonance, fParamU_t& fParamSW) {

	// for 2nd order, K = 2 is max so limit it there
	float K = resonance;
	if(K > 2.0)
		K = 2.0;

	// prewarp for BZT
	double wd = 2 * pi * cutoff;
	double T  = 1 / static_cast<double>(sr);
	double wa = (2/T) * tan(wd * T/2);
	double g  = wa * T/2;

	// G - the feedforward coeff in the VA One Pole
	float G = g / (1.0 + g);

	// the allpass G value
	float GA = 2.0 * G-1;

	//fParamSW.type = 0;		// type Moog Half-Ladder
	fParamSW.K = K;
	fParamSW.G = G;
	fParamSW.alpha0 = 1.0 / (1.0 + K * GA * G * G);
	fParamSW.beta1 = GA * G / (1.0 + g);
	fParamSW.beta2 = GA / (1.0 + g);
	fParamSW.beta3 = 2.0 / (1.0 + g);
	fParamSW.beta4 = 0;						// not used for Moog Half-Ladder

	std::cout << std::endl << "TB_G: " << G << std::endl;
	std::cout << "TB_K: " << K << std::endl;
	//std::cout<<"fAlpha0 = "<<fAlpha0<<std::endl;
	//std::cout<<"setFcAndRes_Ref: g = "<<g<<",        G = "<<G<<",        K = "<<K<<",        beta1 = "<<fBeta1<<",        beta2 = "<<fBeta2<<",        beta3 = "<<fBeta3<<std::endl;

}


void setFcAndRes_ML4P(float sr, float cutoff, float resonance, fParamU_t& fParamSW)

{
	// for 2nd order, K = 2 is max so limit it there
	// ** fixed-point implementation, K=2 requires many integer bits to prevent overflow
	// (future enhancement -> use internal data type to handle bit-growth)
	// 0.0 < G < 1.0
	// currently limit K resonance to less than 2.0 to prevent overflow:
	float K = resonance;
	if(K > 2.0)
		K = 2.0;

	// prewarp for BZT
	double wd = 2 * pi * cutoff;
	double T  = 1 / static_cast<double>(sr);
	double wa = (2/T) * tan(wd * T/2);
	double g  = wa * T/2;

	// G - the feedforward coeff in the VA One Pole
	float G = g / (1.0 + g);

	// fParamSW.type = 1;		// type Moog Ladder 4 pole
	fParamSW.K = K;
	fParamSW.G = G;
	fParamSW.alpha0 = 1.0 / (1.0 + K*G*G*G*G);
	fParamSW.beta1 = (G*G*G) / (1.0 + g);
	fParamSW.beta2 = (G*G) / (1.0 + g);
	fParamSW.beta3 = G / (1.0 + g);
	fParamSW.beta4 = 1 / (1.0 + g);

//	std::cout << "setFcAndRes_ML4P: G = " << G << ",	K = " << K
//			  << ",	beta1 = " << fParamSW.beta1 << ",	beta2 = " << fParamSW.beta2
//			  << ",	beta3 = " << fParamSW.beta3 << ",	beta4 = " << fParamSW.beta4
//			  << ",	alpha0 = " << fParamSW.alpha0 << std::endl;

}


void setFParamHW_ML4P(fParamU_t fParam_SW, fltCtrl_t& K_HW, fltCtrl_t& G_HW, fParam_t& fParam_HW)
{
	K_HW = static_cast<fltCtrl_t>(fParam_SW.K);
	G_HW = static_cast<fltCtrl_t>(fParam_SW.G);
	fParam_HW.type = 1;	// set to 1 for ML4P
	fParam_HW.alpha0 = (fltCtrl_t)fParam_SW.alpha0;
	fParam_HW.beta1 = (fltCtrl_t)fParam_SW.beta1;
	fParam_HW.beta2 = (fltCtrl_t)fParam_SW.beta2;
	fParam_HW.beta3 = (fltCtrl_t)fParam_SW.beta3;
	fParam_HW.beta4 = (fltCtrl_t)fParam_SW.beta4;
}


// decode the Q value; Q on UI is 1->10
float xodSEMSVF_setQ_ref(float Q)
{
	// this maps dQControl = 1->10 to Q = 0.5->25
	float Qscaled = (25.0 - 0.5) * (Q - 1.0) / (10.0 - 1.0) + 0.5;
	return Qscaled;
}


// Re-compute & Update SEMSVF coefficients
void xodSEMSVF_update(float sr, float cutoff, float Q, float& alpha0, float& alpha, float& rho)
{

	// prewarp the cutoff- these are bilinear-transform filters
	float wd = 2 * pi * cutoff;
	float T  = 1 / sr;
	float wa = (2/T) * tan(wd * T/2);
	float g  = wa * T/2;

	// note R is the traditional analog damping factor
	float R = 1.0 / (2.0 * Q);

	// set the coeffs
	alpha0 = 1.0 / (1.0 + 2.0 * R * g + g*g);
	alpha = g;
	rho = 2.0 * R + g;

	//cout<<endl<<"TB odmkSEMSVF_update rho: "<<rho<<endl;

}


// *--------------------------------------------------------------------------------* //
// __lfoFreqCtrl__

lfoFreq_t lfoFreq2phaseStep(float sr, double outputFreq)
{
	// General case:
	// DDS Phase-Step: phaseStepRaw = outputFreq * pow(2, lfoPhaseWidth) / sr
	// * freq CTRL port (phaseStep) uses subset of Full Accumulator width
	// phaseStep = static_cast<lfoFreq_t>(phaseStepRaw / lfoPhaseScale);
	// * lfoPhaseScale shifts usable range of Accumulator width to allow for higher freq when necessary
	//
	// ** 24 bit ddsCtrl range checks **
	// Limiting Factor: 24 bit unsigned Phase-Step CTRL parameter = 16777215 MAX
	//
	// Assume lfoPhaseScale = 1, lfoPhaseWidth = 28, sr = 48000
	// phaseStepRaw = outputFreq * (2**28 / 48000)
	// phaseStep = phaseStepRaw / lfoPhaseScale
	// 16777215 = (outputFreq * (2**28 / 48000)) / lfoPhaseScale
	// .: ((outputFreq * (2**28 / 48000)) / lfoPhaseScale) < 16777215
	// .: outputFreq = 16777215 * lfoPhaseScale / (2**28 / 48000)
	// .: outputFreq < 16777215 * 1 / (2**28 / 48000)
	// .: outputFreq < 3000
	//
	//
	// ** 16 bit ddsCtrl range checks **
	// Limiting Factor: 16 bit unsigned Phase-Step CTRL parameter = 65535 MAX
	//
	// Assume lfoPhaseScale = 1, lfoPhaseWidth = 28, sr = 48000
	// phaseStepRaw = outputFreq * (2**28 / 48000)
	// phaseStep = phaseStepRaw / lfoPhaseScale
	// 65535 = (outputFreq * (2**28 / 48000)) / lfoPhaseScale
	// .: ((outputFreq * (2**28 / 48000)) / lfoPhaseScale) < 65535
	// .: outputFreq = 65535 * lfoPhaseScale / (2**28 / 48000)
	// .: outputFreq < 65535 * 1 / (2**28 / 48000)
	// .: outputFreq < 11.7186
	//
	// ** change lfoPhaseScale = 7
	// .: outputFreq < 65535 * 7 / (2**28 / 48000)
	// .: outputFreq < 82.03
	//
	// assert(outputFreq < ((pow(2, lfoFreqCtrlWidth) - 1) / (pow(2, lfoPhaseWidth) / sr) / lfoPhaseScale));

	assert(outputFreq < (pow(2, lfoFreqCtrlWidth) - 1) * lfoPhaseScale / (pow(2, lfoPhaseWidth) / sr));
	assert(outputFreq > 0);
	double phaseStepRaw = outputFreq * pow(2, lfoPhaseWidth) / sr;
	lfoFreq_t phaseStep = static_cast<lfoFreq_t>(phaseStepRaw);

//	cout << endl << "lfoFreq2phaseStep: lfoOutFreq = " << outputFreq
//		 << ",    ddsStepRaw = " << phaseStepRaw
//		 << ",    ddsStepVal = " << static_cast<float>(phaseStep) << endl;

	return phaseStep;
}


uint16_t lfoFreq2phaseStep16b(float sr, double outputFreq)
{
	// ** 16 bit ddsCtrl range checks **
	// Limiting Factor: 16 bit unsigned Phase-Step CTRL parameter = 65535 MAX
	//
	// General case:
	// DDS Phase-Step: phaseStepRaw = outputFreq * pow(2, lfoPhaseWidth) / sr
	// * freq CTRL port (phaseStep) uses subset of Full Accumulator width
	// phaseStep = static_cast<lfoFreq_t>(phaseStepRaw / lfoPhaseScale);
	//
	// Assume lfoPhaseScale = 1, lfoPhaseWidth = 28, sr = 48000
	// phaseStepRaw = outputFreq * (2**28 / 48000)
	// phaseStep = phaseStepRaw / lfoPhaseScale
	// 65535 = (outputFreq * (2**28 / 48000)) / lfoPhaseScale
	// .: outputFreq * ((2**28 / 48000) / lfoPhaseScale) < 65535
	// .: outputFreq = 65535 * lfoPhaseScale / (2**28 / 48000)
	// .: outputFreq < 65535 * 1 / (2**28 / 48000)
	// .: outputFreq < 11.71857
	//
	// assert(outputFreq < ((pow(2, lfoFreqCtrlWidth) - 1) / (pow(2, lfoPhaseWidth) / sr) / lfoPhaseScale));

	assert(outputFreq < 11.7 && outputFreq > 0);
	double phaseStepRaw = outputFreq * pow(2, lfoPhaseWidth) / sr;
	uint16_t phaseStep = static_cast<uint16_t>(phaseStepRaw / lfoPhaseScale);

//	cout << endl << "lfoFreq2phaseStep16b: lfoOutFreq = " << outputFreq
//		 << ",    ddsStepRaw = " << phaseStepRaw
//		 << ",    ddsStepVal = " << static_cast<float>(phaseStep) << endl << endl;

	return phaseStep;
}

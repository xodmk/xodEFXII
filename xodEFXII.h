/*------------------------------------------------------------------------------------------------*/
/* ___::((xodEFXII.h))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018))::___
   ___::((created by eschei))___

	Purpose: header .h for XODMK EFXII
	Device: All
	Revision History: September 1, 2017 - initial
	Revision History: May 9, 2018 - v4
	Revision History: 2022-01-21 -  v4 xodmk
	Revision History: 2022-09-21 - Multi-Channel template
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#ifndef __XODEFXII_H__
#define __XODEFXII_H__

#include <stdint.h>
#include <math.h>
#include <cassert>
#include <utility>
#include <ap_fixed.h>
#include "ap_shift_reg.h"
#include <hls_stream.h>
#include <ap_axi_sdata.h>


#include "xodEFXII_types.h"

using namespace std;

/*------------------------------------------------------------------------------------------------*/

extern void xodEFXII_top(uint32_t* axiCtrl,					// axi4lite 32bit bus
					   ddlGainBus_t ddlGainBusCtrl,			// <ddlFBGain[31:16], ddlWetDryMix[15:0]> (GPIO x 1)
					   ddlLength4CHAXI_t ddlLengthCtrl,		// <ddlLengthCH3[95:64], ddlLengthCH3[63:32], ddlLengthCH3[31:0]> (3x32 GPIOx2)
					   ddsFreq4CHAXI_t ssbModFreqCtrl,		// <ddsFreqCtrlCH4[127:96], ddsFreqCtrlCH4[95:64], ddsFreqCtrlCH4[63:32], ddsFreqCtrlCH4[31:0]> (4x32 GPIOx2)
					   wavshaperCtrl_t wavShaperGain,		// wavShaperGain[15:0]
					   lfoWavBus_t lfoWavBusCtrl,				// <lfoFreqCtrl[15:0] x 2, lfoGainCtrl[15:0] x 2> (GPIO x 2)
					   uint16_t efxOutGainCtrl,				// efxOutGainCtrl[15:0] (ddlOutputGain)
					   axis_t &efxInAxis,
					   axis_t &efxOutAxis);

/*------------------------------------------------------------------------------------------------*/


// *--------------------------------------------------------------* //
///// Proper-1 Range (1p0) data scaling & conversion

//
template <class T, int nBits>
T floatToUintx_1p0(float din);

template <class T, int nBits>
float uintxToFloat_1p0(T din);

// uintx type to ap_ufixed type
template <class U, class T, int nBits>
T uintxToUFixed_1p0(U din);
// ap_ufixed type to uintx type
template <class T, class U, int nBits>
U ufixedToUintx_1p0(T din);


//ap_ufixed<16, 4> uint16ToUFixed_16_4(uint16_t din);
//uint16_t ufixedToUint16_16_4(ap_ufixed<16, 4> din);

// *--------------------------------------------------------------* //


// Scale float +/- 1.0 to unsigned nBits integer type
template <class T, int nBits>
T floatToUintx_1p0(float din)
{
	assert(din >= 0.0f && din <= 1.0);
	int scaleVal = 1 << (nBits - 1);
	ap_ufixed<2*nBits, nBits> dTemp = static_cast<ap_ufixed<2*nBits, nBits> >(din * scaleVal);
	T dout = static_cast<T>(dTemp + static_cast<ap_ufixed<2, 1> >(0.5));
	return dout;
};

// Scale unsigned nBits integer type to float +/- 1.0
template <class U, int nBits>
float uintxToFloat_1p0(U din)
{
	ap_ufixed<2*nBits, nBits> dTemp = static_cast<ap_ufixed<2*nBits, nBits> >(din);
	float dout = static_cast<float>(dTemp >> (nBits - 1));
	return dout;
}

// Converts a unsigned nBits integer type to ap_fixed<n, 1> +/- 1.0
template <class U, class T, int nBits>
T uintxToUFixed_1p0(U din) {
	ap_ufixed<2*nBits, nBits> dTemp = static_cast<ap_ufixed<2*nBits, nBits> >(din);
	T dout = static_cast<T>(dTemp >> (nBits - 1));
	return dout;
}

// Converts a ap_ufixed<n, 1> [0.0:1.0] / float[0.0:1.0] to unsigned nBits integer type
template <class T, class U, int nBits>
U ufixedToUintx_1p0(T din)
{
	assert(din >= static_cast<T>(0.0) && din <= static_cast<T>(1.0));
	int scaleVal = 1 << (nBits - 1);
	ap_ufixed<2*nBits, nBits> dTemp = static_cast<ap_ufixed<2*nBits, nBits> >(din * scaleVal);
	U dout = static_cast<U>(dTemp + static_cast<ap_ufixed<2, 1> >(0.5));
	return dout;
}


//// Converts and Scales a uint16_t integer type to ap_ufixed<16, 4>
//ap_ufixed<16, 4> uint16ToUFixed_16_4(uint16_t din)
//{
//	ap_ufixed<28, 16> dTemp = static_cast<ap_ufixed<28, 16> >(din);
//	ap_ufixed<16, 4> dout = static_cast<ap_ufixed<16, 4> >(dTemp >> 12);
//	return dout;
//}
//
//
//// Converts and Scales a uint16_t integer type to ap_ufixed<16, 4>
//uint16_t ufixedToUint16_16_4(ap_ufixed<16, 4> din)
//{
//	ap_ufixed<28, 16> dTemp = static_cast<ap_ufixed<28, 16> >(din);
//	uint16_t dout = static_cast<uint16_t>(dTemp << 12);
//	return dout;
//}


// *--------------------------------------------------------* //
// *--- linear interpolation ---* //

template <class T, class F>
T linInterpolate(T dataLow, T dataHigh, F fracDelay)
{
	T linInterpOut;
	linInterpOut = dataLow + (dataHigh - dataLow) * fracDelay;
	return linInterpOut;
}

/*------------------------------------------------------------------------------------------------*/

// *-----------------------------------------------------------------------------------* //
///// Multi-Channel Fir filter used for Hilbert Transform /////////////////////

template <class T, int nChannels>
class MultiChannelFir {

	acc_t acc[nChannels];
	acc_t innerMpy[nChannels];

public:
	MultiChannelFir() {
		for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
			acc[c] = 0;
		}
	}

	void advance(T firIn[nChannels], T firOut[nChannels]);

};


template <class T, int nChannels>
void MultiChannelFir<T, nChannels>::advance(T firIn[nChannels], T firOut[nChannels]) {
//#pragma HLS PIPELINE

	static ap_shift_reg<firData_t, numCoef> shiftReg[nChannels];
	static firData_t shiftData[nChannels] {0};

	for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
		acc[c] = 0;
	}

	Shift_Accum_Loop: for (int i = numCoef-1; i >= 0; i--) {
#pragma HLS UNROLL factor=1
//#pragma HLS LOOP_TRIPCOUNT min=1 max=numCoef avg=numCoef/2
//#pragma HLS PIPELINE

		for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
			if (i == 0) {
				shiftData[c] = shiftReg[c].shift(firIn[c], i);
			} else {
				shiftData[c] = shiftReg[c].read(i);
			}

			innerMpy[c] = shiftData[c] * hilbertCoef[i];
			acc[c] += innerMpy[c];

		}
	}

	for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
		// *** FIXIT *** assertion failes - need to control overflow
		//assert(acc[c] <= 1 && acc[c] >= -1);
		if (acc[c] > 1) acc[c] = 1;
		if (acc[c] < -1) acc[c] = -1;

		firOut[c] = static_cast<firData_t>(acc[c]);
	}

}


// *-----------------------------------------------------------------------------------* //
///// SinCos function /////////////////////

template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
class SinCos {
	T Lut[1<<lutAddrWidth];
	ap_uint<2> taylorOrder;

public:
	SinCos();
	~SinCos() {}
	T get(ap_uint<lutAddrWidth + 1> x);
	pair<T, T> sincos_lut(ap_uint<lutAddrWidth + 2> x);
	pair<T, T> sin_taylor(ap_uint<phaseWidth> x);
};


template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
SinCos<T, dataWidth, phaseWidth, lutAddrWidth>::SinCos() {
//#pragma HLS RESOURCE variable=Lut latency=2

	int lutTableSize = 1 << lutAddrWidth;

	taylorOrder = (dataWidth/(lutAddrWidth + 2) < 3) ? (dataWidth/(lutAddrWidth + 2)): 3;
	//std::cout<<"taylor order = "<<taylorOrder<<std::endl;

	ap_uint<lutAddrWidth + 1> phaseNormCoef = lutTableSize;

	for (int i = 0; i < lutTableSize; i++) {
		Lut[i] = sin(M_PI / (2 * phaseNormCoef) * i);
		//std::cout<<"Lut["<<i<<"]="<<Lut[i]<<std::endl;
	}
}


template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
T SinCos<T, dataWidth, phaseWidth, lutAddrWidth>::get(ap_uint<lutAddrWidth + 1> x) {
	if(x & (1 << lutAddrWidth))
		return 1;
	else
		return Lut[x & ((1<<lutAddrWidth)-1)];
}


template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
pair<T, T> SinCos<T, dataWidth, phaseWidth, lutAddrWidth>::sincos_lut(ap_uint<lutAddrWidth + 2> x) {

	pair<T, T> res;
	ap_uint<lutAddrWidth> mx;

	mx = x & ((1 << lutAddrWidth) - 1);

	int lutTableSize = 1 << lutAddrWidth;

	if (x < lutTableSize) {
		res.first = get(mx);
		res.second = get(lutTableSize - mx);
	} else if (x >= lutTableSize && x < 2 * lutTableSize) {
		res.first = get(lutTableSize - mx);
		res.second = -get(mx);
	} else if (x >= 2 * lutTableSize && x < 3 * lutTableSize) {
		res.first = -get(mx);
		res.second = -get(lutTableSize - mx);
	} else {
		res.first = -get(lutTableSize - mx);
		res.second = get(mx);
	}

	return res;
}


template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
pair<T, T> SinCos<T, dataWidth, phaseWidth, lutAddrWidth>::sin_taylor(ap_uint<phaseWidth> x)
{
	pair<T, T> f0, f1, f2;
	pair<T, T> res;

	ap_uint<phaseWidth> x0;

	const ap_uint<phaseWidth + 1> phaseNormCoef=ap_uint<phaseWidth + 1>(1)<<phaseWidth;
	const ap_uint<64>  coef_2_Pi = (1LL << 59) * 2 * M_PI;

	f0 = sincos_lut(x/(phaseNormCoef>>(lutAddrWidth + 2)));

	x0 = x / (phaseNormCoef >> (lutAddrWidth + 2));
	x0 *= (phaseNormCoef >> (lutAddrWidth + 2));

	ap_uint<phaseWidth + 64> tmp = coef_2_Pi * (x - x0);
	ap_ufixed<65, 65> tmp1 = tmp / phaseNormCoef;

	ap_fixed<dataWidth, 2> delta;
	delta.range(dataWidth - 1, 0)=tmp1.range(61, 61 - dataWidth);

    /*std::cout<<"Correct delta="<<delta1<<", approximate delta="<<(double)delta<<
    		"\t\t|\ttmp="<<(double)tmp<<", tmp1="<<(double)tmp1<<std::endl;*/


	f1.first = f0.first + f0.second * delta;
	f1.second = f0.second - f0.first * delta;

	f2.first = f1.first - f0.first * delta * delta / 2;
	f2.second = f1.second - f0.second * delta * delta / 2;

	//std::cout<<(double)(f0.first*delta*delta/2)<<", delta="<<(double)(delta*delta)<<std::endl;

	if(taylorOrder == 0) {
		res.first = f0.first;
		res.second = f0.second;
	} else if(taylorOrder == 1) {
		res.first = f1.first;
		res.second = f1.second;
	} else if(taylorOrder == 2) {
		res.first = f2.first;
		res.second = f2.second;
	}

	return res;

}


// *-----------------------------------------------------------------------------------* //
///// complex DDS /////////////////////


template <class T, class C, class P, int nChannels, int dataWidth, int phaseWidth, int phaseScale, int lutAddrWidth>
class DDSXN {
	P phase[nChannels];		// P - full phase Accumulator width
	C step[nChannels];		// oscillator phase step - lower rez (ex. 24b) -> cast to full phase Accumulator width
	C offset[nChannels];	// osc channel waveform offset

	SinCos<T, dataWidth, phaseWidth, lutAddrWidth> sc[nChannels];

public:
	DDSXN() {
		for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
			step[c] = 1; //Just a random number
			offset[c] = 0;
		}
	}

	void reset() {
		for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
			phase[c] = 0;
#pragma HLS ARRAY_PARTITION variable = phase complete dim = 0
		}
	}

	void set_step(C phaseStep[nChannels])
	{
		for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
			step[c] = phaseStep[c];
#pragma HLS ARRAY_PARTITION variable = step complete dim = 0
		}
	}

	void set_offset(C xoffset[nChannels])
	{
		for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
			offset[c] = xoffset[c];
#pragma HLS ARRAY_PARTITION variable = offset complete dim = 0
		}
	}

	void advance(pair<T, T> sinCosOut[nChannels]);

};


template <class T, class C, class P, int nChannels, int dataWidth, int phaseWidth, int phaseScale, int lutAddrWidth>
void DDSXN<T, C, P, nChannels, dataWidth, phaseWidth, phaseScale, lutAddrWidth>::advance(pair<T, T> sinCosOut[nChannels]) {

	ap_uint<phaseWidth> scTaylorPhase [nChannels] {0};

	for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
		scTaylorPhase[c] = static_cast<P>(phase[c] + offset[c]);

		sinCosOut[c] = sc[c].sin_taylor(scTaylorPhase[c]);

		// phaseScale: scale lower rez freq control (step value) to full accumulator width
		// phaseScale used to select range of bits => selects scaled freq range
		// should be mirrored in cpu/testbench code when converting freq to phaseStep
		phase[c] += static_cast<P>(step[c] * phaseScale);
	}

}


// *-----------------------------------------------------------------------------------* //
///// Single Side-band Hilbert Transform Modulator /////////////////////

template <int nChannels, int ddsDataWidth, int ddsPhaseWidth, int ddsPhaseScale, int lutAddrWidth>
class XodSSBMod {

	MultiChannelFir<firData_t, nChannels> hilbertFIR;

	DDSXN<ddsData_t, ddsCtrl_t, ddsPhase_t, nChannels, ddsDataWidth, ddsPhaseWidth, ddsPhaseScale, lutAddrWidth> oscDDS;

	ddsCtrl_t freqCtrl[nChannels];

public:
	XodSSBMod() {
		for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
			freqCtrl[c] = static_cast<ddsCtrl_t>(1.0); //Just a random number
		}
		oscDDS.set_step(freqCtrl);
		oscDDS.reset();
	}

	void advance(ddsCtrl_t freqCtrl[nChannels], ssbData_t ssbIn[nChannels], ssbData_t ssbOut[nChannels]);


};


template <int nChannels, int ddsDataWidth, int ddsPhaseWidth, int ddsPhaseScale, int lutAddrWidth>
void XodSSBMod<nChannels, ddsDataWidth, ddsPhaseWidth, ddsPhaseScale, lutAddrWidth>::advance(ddsCtrl_t freqCtrl[nChannels],
																			  	  	  	  	 ssbData_t ssbIn[nChannels],
																							 ssbData_t ssbOut[nChannels]) {
//#pragma HLS PIPELINE

	ssbData_t ssbDlyData1st[nChannels];
	ssbData_t filterData1st[nChannels];
	ssbData_t ssbDlyData2nd[nChannels];
	ssbData_t filterData2nd[nChannels];

	// FIR data
	firData_t firIn[nChannels];
	firData_t firOut[nChannels];

	// delay path (aligns with center FIR tap)
	static ap_shift_reg<ssbData_t, centerTapDly> shDelay[nChannels];

	// oscillator data
	bool reset;
	ddsCtrl_t ddsOffset[nChannels] {0};
	pair<ddsData_t, ddsData_t> ddsSinCosOut[nChannels];
	cmplx_t ddsCmplxOut[nChannels];

	//modulator data
	//*implicit truncation of precision by type assignment - verify HW is correct)
	ssbData_t modulatedDataDelay[nChannels];
	ssbData_t modulatedDataFilter[nChannels];
	ssbData_t mixedData[nChannels];

	// run DDS oscillator to generate modulation
	oscDDS.set_step(freqCtrl);
	oscDDS.advance(ddsSinCosOut);


	for (int c = 0; c < static_cast<int>(nChannels); c++) {
#pragma HLS UNROLL
		ddsCmplxOut[c].real = ddsSinCosOut[c].first;
		ddsCmplxOut[c].imag = ddsSinCosOut[c].second;

		// synchronize internal datapaths
		// *** FIXIT - take care of worst case range ***
		//assert(static_cast<float>(ssbIn[c]) < 1.0);
		//assert(ssbIn[c] < static_cast<ssbData_t>(1.0));
		filterData1st[c] = ssbIn[c];
		ssbDlyData1st[c] = ssbIn[c];

		// Filter and scale input data
		assert(filterData1st[c] == static_cast<firData_t>(filterData1st[c]));
		firIn[c] = static_cast<firData_t>(filterData1st[c]);
	}

	hilbertFIR.advance(firIn, firOut);

	for (int c = 0; c < static_cast<int>(nChannels); c++) {
#pragma HLS UNROLL
		// *** FIXIT - take care of worst case range ***
		//assert(static_cast<float>(firOut[c]) < 1.0 && static_cast<float>(firOut[c]) > -1.0);

		// repack the delayed and filtered data into single struct
		ssbDlyData2nd[c] = shDelay[c].shift(ssbDlyData1st[c], centerTapDly-1);

		filterData2nd[c] = firOut[c];

		// modulate delayed and filtered data
		modulatedDataDelay[c] = ddsCmplxOut[c].real * ssbDlyData2nd[c];
		modulatedDataFilter[c] = -ddsCmplxOut[c].imag * filterData2nd[c];

		// *** FIXIT - take care of worst case range ***
		//assert(static_cast<float>(modulatedDataDelay[c]) < 1.0);
		//assert(static_cast<float>(modulatedDataFilter[c]) < 1.0);

		mixedData[c] = modulatedDataDelay[c] + modulatedDataFilter[c];

		//if (static_cast<float>(mixedData.dataCH1) >= 1.0 || static_cast<float>(mixedData.dataCH2) >= 1.0) {
	//	if (static_cast<float>(mixedData.dataCH2) >= 1.0) {
	//		std::cout << "modulatedData.delayCH2 = " << static_cast<float>(modulatedData.delayCH2)
	//				  << ",   modulatedData.filterCH2 = " << static_cast<float>(modulatedData.filterCH2)
	//				  << ",   mixedData.dataCH2 = " << static_cast<float>(mixedData.dataCH2)
	//				  << std::endl;
	//	}

		// *** FIXIT - take care of worst case range ***
		//assert(static_cast<float>(mixedData.dataCH1) < 1.0 && static_cast<float>(mixedData.dataCH2) < 1.0);
		if (mixedData[c] > static_cast<ssbData_t>(1.0)) {
			ssbOut[c] = static_cast<ssbData_t>(1.0);
		} else if (mixedData[c] < static_cast<ssbData_t>(-1.0)) {
			ssbOut[c] = static_cast<ssbData_t>(-1.0);
		} else {
			ssbOut[c] = mixedData[c];
		}


		//fprintf(stdout,"firIn.dataCH1: %10.7f \t firOut.dataCH1: %10.7f \n", firIn.dataCH1.to_double(), firOut.dataCH1.to_double() );
		//fprintf(stdout,"ssbOut CH1: %10.7f \t mixedData CH1: %10.7f \t modulatedData dly CH1: %10.7f \t modulatedData flt CH1: %10.7f \t oscOut imag: %10.7f \t ssbData2 CH1: %10.7f \n", ssbOut.dataCH1.to_double(), mixedData.dataCH1.to_double(), modulatedData.delayCH1.to_double(), modulatedData.filterCH1.to_double(), oscOut.imag.to_double(), ssbData2.delayCH1.to_double() );
		//fprintf(stdout,"ssbOut CH2: %10.7f \t mixedData CH2: %10.7f \t modulatedData dly CH2: %10.7f \t modulatedData flt CH2: %10.7f \t oscOut real: %10.7f \t ssbData2 CH2: %10.7f \n", ssbOut.dataCH2.to_double(), mixedData.dataCH2.to_double(), modulatedData.delayCH2.to_double(), modulatedData.filterCH2.to_double(), oscOut.real.to_double(), ssbData2.filterCH2.to_double() );
		//if (i < 32) fprintf(stdout,"signal[%4d]=%10.5f \t reference[%4d]=%10.5f\n", i, signal[i].to_double(), i, reference[i].to_double() );

	}	// end channel loop

}

// *-----------------------------------------------------------------------------------* //


#endif

/*------------------------------------------------------------------------------------------------*/
/* ___::((SinCos.h))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018:2022))::___
   ___::((created by eschei))___

	Purpose: Sine / Cosine Look-up table IP
	Device: All
	Revision History: Feb 08, 2017 - initial
	Revision History: May 12, 2018 - ssbMod update
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*

HLS - C++ for FPGA
Sine Look-up table (currently only outputs sine)
Quarter wave symmetry is used for look-up table compression


Taylor Series Noise Correction is used to improve output dynamic range
The order of taylor series is adjusted automatically according to the ratio of output datawidth
and table size (dataWidth, lutAddrWidth). Table size is defined by the table address width + 2 to
account for quarter wave symmetry. the taylor order is calculated as follows:

taylorOrder = (dataWidth/(lutAddrWidth+2) < 3) ? (dataWidth/(lutAddrWidth+2)): 3;

example:
DDS output width = 8, 	sincos LUT address width = 7 => taylorOrder = 0
DDS output width = 17, 	sincos LUT address width = 7 => taylorOrder = 1
DDS output width = 18, 	sincos LUT address width = 7 => taylorOrder = 2
DDS output width = 27, 	sincos LUT address width = 7 => taylorOrder = 3

*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#ifndef __SINCOS_H__
#define __SINCOS_H__

#include <math.h>
#include <utility>

using namespace std;


//const int ddsLutAddrWidth=7;						    // sincos LUT addr width
//// lut_table_size=1<<lut_addr_width;		        // sincos LUT table depth - defined in class
//
//const int ddsDataWidth=24;
//
//// oscStepTest range (36 bits) : [ -34359738368 , 34359738367 ]
//// oscOutFreqTest              : [ -24000       , 23999.9999  ]
//const int ddsPhaseWidth=36;
//
//
//typedef ap_fixed<ddsDataWidth, 2> ddsData_t;			// DDS OSC1 output
////typedef ap_int<ddsPhaseWidth> ddsPhase_t;			// phase increment type
//typedef ap_fixed<ddsPhaseWidth, ddsPhaseWidth> ddsPhase_t;			// phase increment type
//
//
//struct cmplx_t {
//	ddsData_t real;
//	ddsData_t imag;
//};
//
//
//// *--------------------------------------------------------------* //
//
//template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
//class SinCos {
//	T Lut[1<<lutAddrWidth];
//	ap_uint<2> taylorOrder;
//
//public:
//	SinCos();
//	~SinCos() {}
//	T get(ap_uint<lutAddrWidth+1> x);
//	pair<T, T> sincos_lut(ap_uint<lutAddrWidth+2> x);
//	pair<T, T> sin_taylor(ap_uint<phaseWidth> x);
//};
//
//
//template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
//SinCos<T, dataWidth, phaseWidth, lutAddrWidth>::SinCos() {
////#pragma HLS RESOURCE variable=Lut latency=2
//
//	int lutTableSize = 1<<lutAddrWidth;
//
//	taylorOrder = (dataWidth/(lutAddrWidth+2) < 3) ? (dataWidth/(lutAddrWidth+2)): 3;
//	//std::cout<<"taylor order = "<<taylorOrder<<std::endl;
//
//	ap_uint<lutAddrWidth + 1> phaseNormCoef = lutTableSize;
//
//	for (int i = 0; i < lutTableSize; i++) {
//		Lut[i] = sin(M_PI / (2 * phaseNormCoef) * i);
//		//std::cout<<"Lut["<<i<<"]="<<Lut[i]<<std::endl;
//	}
//}
//
//
//template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
//T SinCos<T, dataWidth, phaseWidth, lutAddrWidth>::get(ap_uint<lutAddrWidth+1> x) {
//	if(x & (1<<lutAddrWidth))
//		return 1;
//	else
//		return Lut[x & ((1<<lutAddrWidth)-1)];
//}
//
//
//template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
//pair<T, T> SinCos<T, dataWidth, phaseWidth, lutAddrWidth>::sincos_lut(ap_uint<lutAddrWidth+2> x) {
//
//	pair<T, T> r;
//	ap_uint<lutAddrWidth> mx;
//
//	mx = x & ((1 << lutAddrWidth) - 1);
//
//	int lutTableSize = 1<<lutAddrWidth;
//
//	if (x < lutTableSize) {
//		r.first = get(mx);
//		r.second = get(lutTableSize - mx);
//	} else if (x >= lutTableSize && x < 2 * lutTableSize) {
//		r.first = get(lutTableSize - mx);
//		r.second = -get(mx);
//	} else if (x >= 2 * lutTableSize && x < 3 * lutTableSize) {
//		r.first = -get(mx);
//		r.second = -get(lutTableSize - mx);
//	} else {
//		r.first = -get(lutTableSize - mx);
//		r.second = get(mx);
//	}
//
//	return r;
//}
//
//
//template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
//pair<T, T> SinCos<T, dataWidth, phaseWidth, lutAddrWidth>::sin_taylor(ap_uint<phaseWidth> x)
//{
//	pair<T, T> f0, f1, f2;
//	pair<T, T> r;
//
//	ap_uint<phaseWidth> x0;
//
//	const ap_uint<phaseWidth+1> phaseNormCoef=ap_uint<phaseWidth+1>(1)<<phaseWidth;
//	const ap_uint<64>  coef_2_Pi = (1LL<<59)*2*M_PI;
//
//	f0=sincos_lut(x/(phaseNormCoef>>(lutAddrWidth+2)));
//
//	x0=x/(phaseNormCoef>>(lutAddrWidth+2));
//	x0*=(phaseNormCoef>>(lutAddrWidth+2));
//
//	ap_uint<phaseWidth+64> tmp=coef_2_Pi*(x-x0);
//	ap_ufixed<65,65> tmp1 = tmp/phaseNormCoef;
//
//	ap_fixed<dataWidth,2> delta;
//	delta.range(dataWidth-1, 0)=tmp1.range(61, 61-dataWidth);
//
//    /*std::cout<<"Correct delta="<<delta1<<", approximate delta="<<(double)delta<<
//    		"\t\t|\ttmp="<<(double)tmp<<", tmp1="<<(double)tmp1<<std::endl;*/
//
//
//	f1.first=f0.first+f0.second*delta;
//	f1.second=f0.second-f0.first*delta;
//
//	f2.first=f1.first-f0.first*delta*delta/2;
//	f2.second=f1.second-f0.second*delta*delta/2;
//
//	//std::cout<<(double)(f0.first*delta*delta/2)<<", delta="<<(double)(delta*delta)<<std::endl;
//
//	if(taylorOrder == 0) {
//		r.first=f0.first;
//		r.second=f0.second;
//	} else if(taylorOrder == 1) {
//		r.first=f1.first;
//		r.second=f1.second;
//	} else if(taylorOrder == 2) {
//		r.first=f2.first;
//		r.second=f2.second;
//	}
//
//	return r;
//
//}
//
//
//// *--------------------------------------------------------------* //
//// *--- DDS functions ---*
//
//template <class T, class P, int dataWidth, int phaseWidth, int lutAddrWidth>
//class DDS {
//	P phase;
//	P step;
//	P offset;
//
//	SinCos<T, dataWidth, phaseWidth, lutAddrWidth> sc;
//
//public:
//	DDS() {
//		step = 1; //Just a random number
//		offset = 0;
//	}
//	void reset() { phase = 0; }
//	void set_step(P x)
//	{
//		step = x;
//	}
//	void set_offset(P x)
//	{
//		offset = x;
//	}
//	T advance();
//};
//
//template <class T, class P, int dataWidth, int phaseWidth, int lutAddrWidth>
//T DDS<T, P, dataWidth, phaseWidth, lutAddrWidth>::advance() {
////#pragma HLS RESOURCE variable=phase core=AddSub_DSP
//
//	T res;
//
//	pair<T, T> sinCosOut;
//
//	ap_uint<phaseWidth + 1> phaseNormCoef = 1;
//	phaseNormCoef <<= phaseWidth;
//
//
//	sinCosOut = sc.sin_taylor(phase + offset);
//	phase += step;
//
//	res = sinCosOut.first;
//
//	return res;
//}
//
//
//// *-----------------------------------------------------------------------------------* //
/////// complex DDS /////////////////////
//
//template <int dataWidth=24, int phaseWidth=36, int lutAddrWidth=7>
//class ComplexDDS {
//
////	typedef ap_uint<phaseWidth> phaseType;			// phase increment type
////	typedef ap_fixed<dataWidth,2> dataType;			// DDS OSC1 output
////
////	struct cmplxType {
////		dataType real;
////		dataType imag;
////	};
//
//	ddsPhase_t phase;
//	ddsPhase_t step;
//	ddsPhase_t offset;
//
//	SinCos<ddsData_t, dataWidth, phaseWidth, lutAddrWidth> sc;
//
//public:
//	ComplexDDS() {
//		step = 1; //Just a random number
//		offset = 0;
//
//	}
//
//	void reset() { phase = 0; }
//
//	void set_step(ddsPhase_t x) { step = x; }
//
//	void set_offset(ddsPhase_t x) { offset = x; }
//
//	cmplx_t advance() {
////#pragma HLS RESOURCE variable=phase core=AddSub_DSP
//
//		pair<ddsData_t, ddsData_t> sinCosOut;
//		cmplx_t ddsOut;
//
//		ap_uint<phaseWidth + 1> phaseNormCoef = 1;
//		phaseNormCoef<<=phaseWidth;
//
//		sinCosOut = sc.sin_taylor(phase + offset);
//		ddsOut.real = sinCosOut.first;
//		ddsOut.imag = sinCosOut.second;
//		phase += step;
//
//		return ddsOut;
//	}
//
//};


#endif

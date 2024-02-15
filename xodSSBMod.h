/*------------------------------------------------------------------------------------------------*/
/* ___::((xodSSBMod.h))::___

   ___::((created by eschei))___

	Purpose: header for xodSSBMod - Single Side-band Modulation with Hilbert Transformer
	Device: All
	Revision History: July 14, 2017 - initial
	Revision History: May 12, 2018 - version 2
	Revision History: 2022-02-5 - xodmk
	Revision History: 2022-09-18 - xodmk update with multi-channels
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#include <math.h>
#include <utility>
#include <cassert>

#include <ap_int.h>
#include <ap_fixed.h>
#include "hls_stream.h"
#include "ap_shift_reg.h"

#include "xodEFX.h"


using namespace std;

#ifndef __XODSSBMOD_H__
#define __XODSSBMOD_H__



// *-----------------------------------------------------------------------------------* //
///// definitions /////////////////////

//#define NCHANNELS 4

//const double Fs = 100000000;	// FPGA clock frequency

//const int dataWidth = 24;		// top-level IO data width

// *-----------------------------------------------------------------------------------* //
///// main types /////////////////////

//typedef ap_fixed<dataWidth, 2> ssbData_t;


//// *-----------------------------------------------------------------------------------* //
/////// DDS Typedefs /////////////////////
//
//// ** copied from xodDDS.h
//
////const double Fs = 100000000;	// FPGA clock frequency
//
//const int ddsDataWidth = 24;
//
//// *** current limitation for 24 bit UI frequency control ***
//// to limit ddsCtrl_t to 24 bits for UI, limit max DDS output freq to +/-12KHz
//// * 25 bits ddsCtrl_t required for full 48 KHz sampling (+/-24KHz max DDS output freq)
//// (or - FIXIT - use unsigned ufixed 24 bits for ddsCtrl & add separate negative freq logic)
//// (or - FIXIT - implement DDS freq equation -> requires 'power of 2' in FPGA)
//const int ddsControlWidth = 24;
//const int ddsPhaseWidth = 36;									// high resolution phase accumulator width
//const int lutAddrWidth = 7;						    			// sincos LUT addr width
//const int taylorOrder = 1;
//
//typedef ap_fixed<ddsControlWidth, ddsControlWidth> ddsCtrl_t;	// DDS control data type
//typedef ap_fixed<ddsPhaseWidth, ddsPhaseWidth> ddsPhase_t;		// phase increment type
//typedef ap_fixed<ddsDataWidth, 2> ddsData_t;
//
////using ddsMultiPhase_t = ddsPhase_t [NCHANNELS];
////using ddsMultiData_t = pair<ddsData_t, ddsData_t> [NCHANNELS];
//
//struct cmplx_t {
//	ddsData_t real;
//	ddsData_t imag;
//};


// *-----------------------------------------------------------------------------------* //
///// filter data types /////////////////////


// **assume using DSP48e1 18x25 Mpy**

const unsigned int firDataWidth = dataWidth;
const unsigned int coefWidth = 18;
const unsigned int firBitGrowth = 2;			// from pythonFIR.py

typedef ap_fixed<coefWidth, 2>	coef_t;
typedef ap_ufixed<coefWidth, 2>  gain_t;

//typedef ap_fixed<firDataWidth, 2>	firData_t;
//typedef ap_fixed<firDataWidth+coefWidth, 2 + firBitGrowth>	acc_t;

// ** Using extended data types **
typedef ap_fixed<firDataWidth+8, 6>	firData_t;
typedef ap_fixed<firDataWidth+coefWidth+8, 6 + firBitGrowth>	acc_t;


// 369 tap Hilbert FIR - designed in Python
const unsigned int numCoef = 369;				// should match pythonFIR.py parameters
const unsigned int centerTapDly = 184;			// position of center FIR Coefficient
//const gain_t firGain = 2.63448870847;
//const gain_t firGainInv = 0.37958029456909603;


// *-----------------------------------------------------------------------------------* //
///// hilbert FIR coefficient /////////////////////


/* FIR Coefficients generated using Python+Scipy (ssbHilbertMod.py) :
 * fHilbert=sp.signal.remez(99, [0.03, 0.47],[1], type='hilbert'); */

// Example - current design reads coefficients from .dat file
const coef_t hilbertCoef[numCoef] = {
		-0.0000027971672683,    0.0005668243045958,    0.0000021582674063,    0.0001440592899448,
		-0.0000000428581538,    0.0001622322743151,    -0.0000001042761430,    0.0001818708019650,
		-0.0000001607895767,    0.0002029979926012,    -0.0000001809999381,    0.0002256499142878,
		-0.0000001449098678,    0.0002498371451223,    -0.0000000796812276,    0.0002756082372421,
		-0.0000000506216023,    0.0003031217699056,    -0.0000001073899354,    0.0003326199643021,
		-0.0000002129872611,    0.0003643020827049,    -0.0000002052287741,    0.0003980697354330,
		0.0000000278900410,    0.0004335176928815,    0.0000001614927666,    0.0004708128391678,
		-0.0000004172634266,    0.0005115933884211,    0.0000000176787297,    0.0005536546622183,
		-0.0000001849469051,    0.0005986339809044,    -0.0000001371644143,    0.0006460536769266,
		-0.0000001215104853,    0.0006961208478517,    -0.0000001442285457,    0.0007489641395459,
		-0.0000001957557257,    0.0008047153456476,    -0.0000002180297631,    0.0008634755835951,
		-0.0000001665246477,    0.0009253245232310,    -0.0000000903703109,    0.0009903630339033,
		-0.0000001238926573,    0.0010587584496290,    -0.0000002408342631,    0.0011306657168229,
		-0.0000002010973113,    0.0012061247054804,    -0.0000000771212112,    0.0012852483193891,
		-0.0000002892183437,    0.0013683155959127,    -0.0000001435241686,    0.0014553239574353,
		-0.0000002449177181,    0.0015465017506511,    -0.0000002655054130,    0.0016419839245538,
		-0.0000002654202343,    0.0017419529101161,    -0.0000002384163353,    0.0018465928236920,
		-0.0000002259836788,    0.0019560890778418,    -0.0000002528141455,    0.0020706198898401,
		-0.0000002727517867,    0.0021903836179706,    -0.0000002270017753,    0.0023156250725059,
		-0.0000001904785508,    0.0024465768743472,    -0.0000002349089495,    0.0025834376087981,
		-0.0000002376930646,    0.0027264756485976,    -0.0000001873921894,    0.0028760147192473,
		-0.0000002812905273,    0.0030322200492974,    -0.0000002276024032,    0.0031955621147827,
		-0.0000002502466326,    0.0033662963043093,    -0.0000002606048313,    0.0035448023583031,
		-0.0000002769235368,    0.0037314620501271,    -0.0000002758829953,    0.0039267185318265,
		-0.0000002555715887,    0.0041310754093512,    -0.0000002445359290,    0.0043450290868452,
		-0.0000002483329736,    0.0045690930680355,    -0.0000002293829791,    0.0048039148431990,
		-0.0000001996153404,    0.0050501892299286,    -0.0000002005116227,    0.0053085881592326,
		-0.0000001941043677,    0.0055799597895743,    -0.0000001700015219,    0.0058652866115900,
		-0.0000001878836658,    0.0061653954641234,    -0.0000001765279505,    0.0064815477950166,
		-0.0000001806470858,    0.0068149450004986,    -0.0000001828037111,    0.0071670211602779,
		-0.0000001890179551,    0.0075393491459137,    -0.0000001885961077,    0.0079337018826826,
		-0.0000001828152179,    0.0083521589906808,    -0.0000001766524286,    0.0087970647057614,
		-0.0000001703017550,    0.0092710485425240,    -0.0000001664322980,    0.0097772209309617,
		-0.0000001592767069,    0.0103191830259970,    -0.0000001418591055,    0.0109010210861351,
		-0.0000001292084045,    0.0115276201369778,    -0.0000001351147310,    0.0122048043128822,
		-0.0000001254663478,    0.0129392736083207,    -0.0000001221494056,    0.0137392432812105,
		-0.0000001147145879,    0.0146145135754718,    -0.0000001079311634,    0.0155770652104650,
		-0.0000001114351109,    0.0166416128366098,    -0.0000001018156858,    0.0178264088528772,
		-0.0000001001025044,    0.0191544835985757,    -0.0000001014470290,    0.0206552259780350,
		-0.0000000808453104,    0.0223666966440739,    -0.0000000758199709,    0.0243392053813132,
		-0.0000000955186754,    0.0266405939341388,    -0.0000000870880452,    0.0293644914826645,
		-0.0000000712238573,    0.0326440240251014,    -0.0000000900373882,    0.0366750432945206,
		-0.0000000835031541,    0.0417577850953297,    -0.0000000895148496,    0.0483773634580267,
		-0.0000000888573772,    0.0573717000755485,    -0.0000000764996608,    0.0703236305613659,
		-0.0000000788050472,    0.0906250084654276,    -0.0000000510262652,    0.1270947385235992,
		-0.0000000360010810,    0.2120689964166065,    -0.0000000371645557,    0.6365738967733989,
		0.0000000000000000,    -0.6365738967733989,    0.0000000371645557,    -0.2120689964166065,
		0.0000000360010810,    -0.1270947385235992,    0.0000000510262652,    -0.0906250084654276,
		0.0000000788050472,    -0.0703236305613659,    0.0000000764996608,    -0.0573717000755485,
		0.0000000888573772,    -0.0483773634580267,    0.0000000895148496,    -0.0417577850953297,
		0.0000000835031541,    -0.0366750432945206,    0.0000000900373882,    -0.0326440240251014,
		0.0000000712238573,    -0.0293644914826645,    0.0000000870880452,    -0.0266405939341388,
		0.0000000955186754,    -0.0243392053813132,    0.0000000758199709,    -0.0223666966440739,
		0.0000000808453104,    -0.0206552259780350,    0.0000001014470290,    -0.0191544835985757,
		0.0000001001025044,    -0.0178264088528772,    0.0000001018156858,    -0.0166416128366098,
		0.0000001114351109,    -0.0155770652104650,    0.0000001079311634,    -0.0146145135754718,
		0.0000001147145879,    -0.0137392432812105,    0.0000001221494056,    -0.0129392736083207,
		0.0000001254663478,    -0.0122048043128822,    0.0000001351147310,    -0.0115276201369778,
		0.0000001292084045,    -0.0109010210861351,    0.0000001418591055,    -0.0103191830259970,
		0.0000001592767069,    -0.0097772209309617,    0.0000001664322980,    -0.0092710485425240,
		0.0000001703017550,    -0.0087970647057614,    0.0000001766524286,    -0.0083521589906808,
		0.0000001828152179,    -0.0079337018826826,    0.0000001885961077,    -0.0075393491459137,
		0.0000001890179551,    -0.0071670211602779,    0.0000001828037111,    -0.0068149450004986,
		0.0000001806470858,    -0.0064815477950166,    0.0000001765279505,    -0.0061653954641234,
		0.0000001878836658,    -0.0058652866115900,    0.0000001700015219,    -0.0055799597895743,
		0.0000001941043677,    -0.0053085881592326,    0.0000002005116227,    -0.0050501892299286,
		0.0000001996153404,    -0.0048039148431990,    0.0000002293829791,    -0.0045690930680355,
		0.0000002483329736,    -0.0043450290868452,    0.0000002445359290,    -0.0041310754093512,
		0.0000002555715887,    -0.0039267185318265,    0.0000002758829953,    -0.0037314620501271,
		0.0000002769235368,    -0.0035448023583031,    0.0000002606048313,    -0.0033662963043093,
		0.0000002502466326,    -0.0031955621147827,    0.0000002276024032,    -0.0030322200492974,
		0.0000002812905273,    -0.0028760147192473,    0.0000001873921894,    -0.0027264756485976,
		0.0000002376930646,    -0.0025834376087981,    0.0000002349089495,    -0.0024465768743472,
		0.0000001904785508,    -0.0023156250725059,    0.0000002270017753,    -0.0021903836179706,
		0.0000002727517867,    -0.0020706198898401,    0.0000002528141455,    -0.0019560890778418,
		0.0000002259836788,    -0.0018465928236920,    0.0000002384163353,    -0.0017419529101161,
		0.0000002654202343,    -0.0016419839245538,    0.0000002655054130,    -0.0015465017506511,
		0.0000002449177181,    -0.0014553239574353,    0.0000001435241686,    -0.0013683155959127,
		0.0000002892183437,    -0.0012852483193891,    0.0000000771212112,    -0.0012061247054804,
		0.0000002010973113,    -0.0011306657168229,    0.0000002408342631,    -0.0010587584496290,
		0.0000001238926573,    -0.0009903630339033,    0.0000000903703109,    -0.0009253245232310,
		0.0000001665246477,    -0.0008634755835951,    0.0000002180297631,    -0.0008047153456476,
		0.0000001957557257,    -0.0007489641395459,    0.0000001442285457,    -0.0006961208478517,
		0.0000001215104853,    -0.0006460536769266,    0.0000001371644143,    -0.0005986339809044,
		0.0000001849469051,    -0.0005536546622183,    -0.0000000176787297,    -0.0005115933884211,
		0.0000004172634266,    -0.0004708128391678,    -0.0000001614927666,    -0.0004335176928815,
		-0.0000000278900410,    -0.0003980697354330,    0.0000002052287741,    -0.0003643020827049,
		0.0000002129872611,    -0.0003326199643021,    0.0000001073899354,    -0.0003031217699056,
		0.0000000506216023,    -0.0002756082372421,    0.0000000796812276,    -0.0002498371451223,
		0.0000001449098678,    -0.0002256499142878,    0.0000001809999381,    -0.0002029979926012,
		0.0000001607895767,    -0.0001818708019650,    0.0000001042761430,    -0.0001622322743151,
		0.0000000428581538,    -0.0001440592899448,    -0.0000021582674063,    -0.0005668243045958,
		0.0000027971672683
};


//// calculate worst case bit-growth
//static double firCoefAcc = 0;
//double firBitGrowth = 0;
//
//for (i=0;i<numCoef;i++) {
//
//	if (hilbertCoef[i] < 0) {
//		firCoefAcc+=(double)-hilbertCoef[i];
//	} else {
//		firCoefAcc+=(double)hilbertCoef[i];
//	}
//	//fprintf(stdout,"taps[%4d]=%10.7f\n", i, firCoef_fx[i].to_double());
//}
//
//firBitGrowth = ceil(log2(firCoefAcc));
//std::cout<<"fir coef accumulation = "<<firCoefAcc<<std::endl;
//fprintf(stdout,"worst case output bit-growth = %f\n",firBitGrowth);


// *-----------------------------------------------------------------------------------* //
///// Stereo Fir filter used for Hilbert Transform /////////////////////

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
		assert(acc[c] <= 1 && acc[c] >= -1);
		//if (acc[c] > 1) acc[c] = 1;
		//if (acc[c] < -1) acc[c] = -1;

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

template <class T, class C, class P, int nChannels, int dataWidth, int phaseWidth, int lutAddrWidth>
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


template <class T, class C, class P, int nChannels, int dataWidth, int phaseWidth, int lutAddrWidth>
void DDSXN<T, C, P, nChannels, dataWidth, phaseWidth, lutAddrWidth>::advance(pair<T, T> sinCosOut[nChannels]) {

	ap_uint<phaseWidth> scTaylorPhase [nChannels] {0};

	for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
		scTaylorPhase[c] = static_cast<P>(phase[c] + offset[c]);

		sinCosOut[c] = sc[c].sin_taylor(scTaylorPhase[c]);

		// cast lower rez freq control (step value) to full accumulator width
		phase[c] += static_cast<P>(step[c]);
	}

}


// *-----------------------------------------------------------------------------------* //
///// Single Side-band Hilbert Transform Modulator /////////////////////

template <int nChannels, int ddsDataWidth, int ddsPhaseWidth, int lutAddrWidth>
class XodSSBMod {

	MultiChannelFir<firData_t, nChannels> hilbertFIR;

	DDSXN<ddsData_t, ddsCtrl_t, ddsPhase_t, nChannels, ddsDataWidth, ddsPhaseWidth, lutAddrWidth> oscDDS;

	ddsCtrl_t freqCtrl[nChannels] {0};

public:
	XodSSBMod() {
		for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
			freqCtrl[c] = 1; //Just a random number
		}
		oscDDS.set_step(freqCtrl);
		oscDDS.reset();
	}

	void advance(ddsCtrl_t freqCtrl[nChannels], ssbData_t ssbIn[nChannels], ssbData_t ssbOut[nChannels]);


};


template <int nChannels, int ddsDataWidth, int ddsPhaseWidth, int lutAddrWidth>
void XodSSBMod<nChannels, ddsDataWidth, ddsPhaseWidth, lutAddrWidth>::advance(ddsCtrl_t freqCtrl[nChannels],
																			  ssbData_t ssbIn[nChannels], ssbData_t ssbOut[nChannels]) {
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
		assert(static_cast<float>(ssbIn[c]) < 1.0);
		assert(ssbIn[c] < static_cast<ssbData_t>(1.0));
		filterData1st[c] = ssbIn[c];
		ssbDlyData1st[c] = ssbIn[c];

		// Filter and scale input data
		assert(filterData1st[c] == static_cast<firData_t>(filterData1st[c]));
		firIn[c] = static_cast<firData_t>(filterData1st[c]);
	}

	hilbertFIR.advance(firIn, firOut);

	for (int c = 0; c < static_cast<int>(nChannels); c++) {
#pragma HLS UNROLL
		assert(static_cast<float>(firOut[c]) < 1.0 && static_cast<float>(firOut[c]) < 1.0);

		// repack the delayed and filtered data into single struct
		ssbDlyData2nd[c] = shDelay[c].shift(ssbDlyData1st[c], centerTapDly-1);

		filterData2nd[c] = firOut[c];

		// modulate delayed and filtered data
		modulatedDataDelay[c] = ddsCmplxOut[c].real * ssbDlyData2nd[c];
		modulatedDataFilter[c] = -ddsCmplxOut[c].imag * filterData2nd[c];
		assert(static_cast<float>(modulatedDataDelay[c]) < 1.0);
		assert(static_cast<float>(modulatedDataFilter[c]) < 1.0);

		mixedData[c] = modulatedDataDelay[c] + modulatedDataFilter[c];

		//if (static_cast<float>(mixedData.dataCH1) >= 1.0 || static_cast<float>(mixedData.dataCH2) >= 1.0) {
	//	if (static_cast<float>(mixedData.dataCH2) >= 1.0) {
	//		std::cout << "modulatedData.delayCH2 = " << static_cast<float>(modulatedData.delayCH2)
	//				  << ",   modulatedData.filterCH2 = " << static_cast<float>(modulatedData.filterCH2)
	//				  << ",   mixedData.dataCH2 = " << static_cast<float>(mixedData.dataCH2)
	//				  << std::endl;
	//	}
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


#endif // __XODSSBMOD_H__

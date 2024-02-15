/*------------------------------------------------------------------------------------------------*/
/* ___::((xodLFOXN.h))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((ODMK:2017:2022))::___
   ___::((created by eschei))___

	Purpose: testbench for Lfo design
	Device: All
	Revision History: Feb 08, 2017 - initial
	Revision History: April 08, 2022 - xodmk update 1
	Revision History: 2022-09-21 - Multi-Channel template
*/

/*------------------------------------------------------------------------------------------------*/
/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*

	** FIXIT FIXIT - move DDSXN params to xodLFOXN.h & test for ALT LFO template parameters **

	** FIXIT FIXIT - WARNING: [HLS 200-880] The II Violation in module 'xodDDSXN' **

	Header for XODMK LFO HLS function 

	SinCos: HLS - C++ for FPGA
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


#ifndef __XODLFOXN_H__
#define __XODLFOXN_H__


#include <math.h>
#include <utility>
#include <ap_int.h>
#include <ap_fixed.h>


#include "xodEFXII_types.h"

using namespace std;


// *--------------------------------------------------------------* //
// *--- SAW functions ---*

template <class T, class C, int nChannels, int phaseWidth, int phaseScale>
class Sawtooth {

	ap_uint<phaseWidth> phase[nChannels];
	C step[nChannels];
	C offset[nChannels];

public:
	Sawtooth() {
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
	void set_step(C x[nChannels])
	{
		for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
			step[c] = x[c];
#pragma HLS ARRAY_PARTITION variable = step complete dim = 0
		}
	}
	void set_offset(C x[nChannels])
	{
		for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
			offset[c] = x[c];
#pragma HLS ARRAY_PARTITION variable = offset complete dim = 0
		}
	}
	void advance(bool upDn, T sawOut[nChannels]);
};


template <class T, class C, int nChannels, int phaseWidth, int phaseScale>
void Sawtooth<T, C, nChannels, phaseWidth, phaseScale>::advance(bool upDn, T sawOut[nChannels]) {

	// ***create up or down saw wave**
	ap_fixed<phaseWidth, 2> sawSH1[nChannels];
	T sawSH2[nChannels];

	for (int c = 0; c < nChannels; c++) {
#pragma HLS UNROLL
		sawSH1[c].range(phaseWidth - 1, 0) = phase[c];
		sawSH2[c] = static_cast<T>(sawSH1[c]);				// this cast discards LSBs
		if (upDn == true) {
			sawOut[c] = -static_cast<T>(sawSH2[c]) >> 1;	// additional shift limits output to +/- 0.9999...
		} else {
			sawOut[c] = static_cast<T>(sawSH2[c]) >> 1;
		}

		phase[c] += static_cast<ap_fixed<phaseWidth, 2> >(step[c] * phaseScale);

	}
}


// *--------------------------------------------------------------* //

extern void xodLfoXN(ap_uint<1> reSync,
				     ap_uint<1> upDn,
					 multiLfoShape_t lfoType,
					 multiLfoFreq_t lfoFreq,
				     multiLfoData_t lfoOut,
				     ap_uint<1> soc[LFOCHANNELS]);


extern void xodLfoMultiXN(ap_uint<1> reSync,
				   	 	  ap_uint<1> upDn,
						  multiLfoFreq_t lfoFreq,
						  multiLfoData_t lfoSinOut,
						  multiLfoData_t lfoSawOut,
						  multiLfoData_t lfoSqrOut,
						  ap_uint<1> soc[LFOCHANNELS]);


// *--------------------------------------------------------------* //


#endif

/*------------------------------------------------------------------------------------------------*/
/* ___::((xodLFOXN.cpp))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((ODMK:2017:2023))::___
   ___::((created by eschei))___

	Purpose: top-level cpp for Lfo function
	Device: All
	Revision History: September 24, 2017 - initial
	Revision History: April 08, 2022     - xodmk update 1
	Revision History: 2023_03_17 	     - xodmk MultiChannel DDS LFO
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*

	Top Level Instantiation of LFO function
	Sin out, Up/Down Saw out, Square Out, Start-Of-Cycle pulse out

	// example conversion of tb frequency to lfoStep value:

	const double Fclk = 100000000;				// This clk should match the actual HW clock
	lorezPhase_t lfoStepTest = 1;					// init

	const double lfoOutFreq_init = 250000.0;
	//const double lfoOutFreq_init = 50000.0;
	//const double lfoOutFreq_init = .560;

	lfoStepTest = lfoOutFreq_init * pow(2, lorezPhaseWidth) / Fclk;

*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#include "xodEFXII_types.h"
#include "xodLFOXN.h"
#include "xodEFXII.h"


extern void xodLfoXN(ap_uint<1> reSync,
				     ap_uint<1> upDn,
					 multiLfoShape_t lfoType,
					 multiLfoFreq_t lfoFreq,
				     multiLfoData_t lfoOut,
				     ap_uint<1> soc[LFOCHANNELS])
{

#pragma HLS PIPELINE

	static DDSXN<lfoData_t, lfoFreq_t, lfoPhase_t, static_cast<int>(LFOCHANNELS), lfoDataWidth, lfoPhaseWidth, lfoPhaseScale, lfoLutAddrWidth> lfoDds;
	static Sawtooth<lfoData_t, lfoFreq_t, static_cast<int>(LFOCHANNELS), lfoPhaseWidth, lfoPhaseScale> lfoSaw;

	pair<lfoData_t, lfoData_t> lfoSinCosOut[LFOCHANNELS];

	multiLfoData_t lfoSawOut {0};
	multiLfoData_t lfoSqrOut {0};
	multiLfoFreq_t lfoOffset {0};

	static bool prevSign[LFOCHANNELS];
	static bool currSign[LFOCHANNELS];

	if(reSync == 1) {
		lfoDds.reset();
		lfoDds.set_offset(lfoOffset);
		lfoSaw.reset();
		for (int c = 0; c < static_cast<int>(LFOCHANNELS); c++) {
#pragma HLS UNROLL
			soc[c] = 1;
			prevSign[c] = true;
		}
	}

	lfoDds.set_step(lfoFreq);
	lfoSaw.set_step(lfoFreq);
	lfoDds.advance(lfoSinCosOut);
	lfoSaw.advance(upDn, lfoSawOut);

	for (int c = 0; c < static_cast<int>(LFOCHANNELS); c++) {
#pragma HLS UNROLL
		if (lfoSinCosOut[c].first >= 0) {
			lfoSqrOut[c] = 1;
		} else {
			lfoSqrOut[c] = -1;
		}

		if (lfoType[c] == 0) {
			if (upDn == 0) {
				lfoOut[c] = lfoSinCosOut[c].first;	// sin for slow ascent
			} else {
				lfoOut[c] = lfoSinCosOut[c].second;	// cos for slow descent
			}
		} else if (lfoType[c] == 1) {
			lfoOut[c] = lfoSawOut[c];
		} else if (lfoType[c] == 2) {
			lfoOut[c] = lfoSqrOut[c];
		} else {
			lfoOut[c] = lfoSinCosOut[c].first;
		}

		currSign[c] = lfoSinCosOut[c].first.is_neg();
		if (prevSign[c] == true && currSign[c] == false) {
			soc[c] = 1;
		} else {
			soc[c] = 0;
		}
		prevSign[c] = currSign[c];
	}

}


extern void xodLfoMultiXN(ap_uint<1> reSync,
				   	 	  ap_uint<1> upDn,
						  multiLfoFreq_t lfoFreq,
						  multiLfoData_t lfoSinOut,
						  multiLfoData_t lfoSawOut,
						  multiLfoData_t lfoSqrOut,
						  ap_uint<1> soc[LFOCHANNELS])
{


#pragma HLS PIPELINE

	static DDSXN<lfoData_t, lfoFreq_t, lfoPhase_t, static_cast<int>(LFOCHANNELS), lfoDataWidth, lfoPhaseWidth, lfoPhaseScale, lfoLutAddrWidth> lfoDds;
	static Sawtooth<lfoData_t, lfoFreq_t, static_cast<int>(LFOCHANNELS), lfoPhaseWidth, lfoPhaseScale> lfoSaw;

	pair<lfoData_t, lfoData_t> lfoSinCosOut[LFOCHANNELS];

	multiLfoFreq_t lfoOffset {0};

	static bool prevSign[LFOCHANNELS];
	static bool currSign[LFOCHANNELS];

	if(reSync == 1) {
		lfoDds.reset();
		lfoDds.set_offset(lfoOffset);
		lfoSaw.reset();
		for (int c = 0; c < static_cast<int>(LFOCHANNELS); c++) {
#pragma HLS UNROLL
			soc[c] = 1;
			prevSign[c] = true;
		}
	}

	lfoDds.set_step(lfoFreq);
	lfoSaw.set_step(lfoFreq);
	lfoDds.advance(lfoSinCosOut);
	lfoSaw.advance(upDn, lfoSawOut);


	for (int c = 0; c < static_cast<int>(LFOCHANNELS); c++) {
#pragma HLS UNROLL

		if (upDn == 0) {
			lfoSinOut[c] = lfoSinCosOut[c].first;	// sin for slow ascent
		} else {
			lfoSinOut[c] = lfoSinCosOut[c].second;	// cos for slow descent
		}
		if (lfoSinOut[c] >= 0) {
			lfoSqrOut[c] = 1;
		} else {
			lfoSqrOut[c] = -1;
		}

		currSign[c] = lfoSinOut[c].is_neg();
		if (prevSign[c] == true && currSign[c] == false) {
			soc[c] = 1;
		} else {
			soc[c] = 0;
		}
		prevSign[c] = currSign[c];

	}

}

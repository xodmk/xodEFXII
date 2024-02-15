/*------------------------------------------------------------------------------------------------*/
/* ___::((xodDDLXN.h))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018))::___
   ___::((created by eschei))___

	Purpose: Multi-Channel Digital Delay C++ HLS
	Device: All
	Revision History: September 1, 2017 - initial
	Revision History: May 9, 2018 - v4
	Revision History: 2022-01-21 -  v4 xodmk
	Revision History: 2022-02-14 -  v5 xodmk
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#ifndef __XODDDL5_H__
#define __XODDDL5_H__

//#include <math.h>
#include <ap_int.h>
#include <cassert>
#include "hls_math.h"
#include "ap_fixed.h"
#include "ap_shift_reg.h"

//#include "xodCrossFader.h"
#include "xodMixMatrix.h"
//#include "xodLfo.h"

#include "xodEFXII_types.h"
#include "xodEFXII.h"


///*------------------------------------------------------------------------------------------------*/
//// *--- DDL Types
//
//#define MAXDELAY 24000
//
////const unsigned DLYBWIDTH = ceil(log2(MAXDELAY));
//const unsigned DLYBWIDTH = 16;
//const unsigned DLYFRACWIDTH = 8;
//
//typedef ap_ufixed<16, 1> delayCtrl_t;
//typedef ap_ufixed<18, 1> delayGain_t;
//typedef ap_uint<DLYBWIDTH> delayInt_t;
//typedef ap_ufixed<DLYBWIDTH+DLYFRACWIDTH, DLYBWIDTH> delay_t;
//typedef ap_ufixed<DLYFRACWIDTH, 0> delayFrac_t;
//
//
//struct ddlCtrlStruct {
//	delay_t delayLength;
//	delayCtrl_t wetMix;
//	delayCtrl_t feedbackMix;
//	delayCtrl_t outputGain;
//};
//
//typedef ddlCtrlStruct ddlCtrlAll_t;


/* ---------------------------------------------------------- */

//template <class T, class F>
//T linInterpolate(T dataLow, T dataHigh, F fracDelay)
//{
//	T linInterpOut;
//	linInterpOut = dataLow + (dataHigh - dataLow) * fracDelay;
//	return linInterpOut;
//}


//# warning note that we can't put delays that are smaller than latencies of the generated HW RTL and those have to be found empirically...
template <typename T, unsigned int est_latency>
class fbShifter {
    unsigned int pointer;
    T feedback[est_latency];
public:
    fbShifter() : pointer(0) {}

    T shift(T value) {
#pragma HLS dependence variable=feedback false
        T t= feedback[pointer];
        feedback[pointer]=value;
        pointer=pointer==est_latency-1? 0 : pointer+1; assert(pointer<est_latency);
        return t;
    }
};


/*------------------------------------------------------------------------------------------------*/

template<int maxChannel, int maxDelay>
class xodDDLXN {

	//CrossFader<data_t, lutAddrWidth> xFader;

	fbShifter<dataExt_t, 1> dlyFeedback[maxChannel]; // appears that any length of shreg works, investigate consequences..

	// internal variables
	//stereoData_t ddlBuffer[MAXDELAY];
	dataExt_t ddlBuffer[maxChannel][maxDelay];
	dataExt_t internalFB[maxChannel];

	//delayLenInt_t readIndex[maxChannel];
	delayLenInt_t readIndexP1[maxChannel];
	delayLenInt_t writeIndex[maxChannel];

	delayLenInt_t delayInteger[maxChannel];
	delayLen_t delayFractional[maxChannel];

	dataExt_t ddlPrevious[maxChannel];
	dataExt_t ddlRead[maxChannel];
	dataExt_t ddlReadP1[maxChannel];
	dataExt_t ddlInterp[maxChannel];
	dataExt_t ddlWet[maxChannel];
	dataExt_t ddlMixed[maxChannel];


	//TEMP PROBE
	int ddlxnChanCnt = 0;
	int ddlxnSampleCnt = 0;


public:

	//ap_uint<1> selectCross;
	//ap_uint<1> useExternalFB;	// 0 = normalized internal FB; 1 = external FB
	//delayCtrl_t wetMix;
	//delayCtrl_t feedbackMix;
	//delayCtrl_t outputGain;

	xodDDLXN() {
		for (int i = 0; i < maxChannel; i++) {
			delayFractional[i] = static_cast<delayLenInt_t>(maxDelay);
			delayInteger[i] = static_cast<delayLenInt_t>(delayFractional[i]);
			//readIndex[i] = 1;
			readIndexP1[i] = 2;
			writeIndex[i] = 0;
		}
	};

	xodDDLXN(ddlLenArray_t ddlLenArray)
	{
		for (int i = 0; i < maxChannel; i++) {
			delayFractional[i] = ddlLenArray[i];
			delayInteger[i] = static_cast<delayLenInt_t>(delayFractional[i]);

			// TEMP PROBE it
			//std::cout << "xodDDL5: r = " << r << ", delayFractional[" << i << "] = " << static_cast<float>(delayFractional[i]) << "\n" << std::endl;

			//readIndex[i] = 1;
			readIndexP1[i] = 2;
			writeIndex[i] = 0;
		}
	}


	void advance(ap_uint<1> selectCross,
				 ap_uint<1> selectExtFB,
				 ddlCtrlAll_t ddlCtrlAll,
				 ddlLenArray_t ddlLenArray,
				 multiData_t ddlIn,
				 multiData_t feedbackIn,
				 multiData_t &feedbackOut,
				 multiData_t &ddlOut);

};


/* ---------------------------------------------------------- */

template<int maxChannel, int maxDelay>
void xodDDLXN<maxChannel, maxDelay>::advance(ap_uint<1> selectCross,
					  	  	  	  	  	     ap_uint<1> selectExtFB,
											 ddlCtrlAll_t ddlCtrlAll,
											 ddlLenArray_t ddlLenArray,
											 multiData_t ddlIn,
											 multiData_t feedbackIn,
											 multiData_t &feedbackOut,
											 multiData_t &ddlOut)
{
#pragma HLS bind_storage variable=ddlBuffer type=RAM_2P impl=BRAM latency=1
//#pragma HLS ARRAY_PARTITION variable=ddlBuffer block factor=2 dim=1
//#pragma HLS ARRAY_PARTITION variable=ddlBuffer block factor=4 dim=2
#pragma HLS DEPENDENCE variable=ddlBuffer inter false
#pragma HLS DEPENDENCE variable=ddlBuffer intra false


#pragma HLS PIPELINE

	assert(ddlIn[0] <=  static_cast<dataExt_t>(1.0) && ddlIn[1] <=  static_cast<dataExt_t>(1.0));
	assert(ddlIn[0] >= static_cast<dataExt_t>(-1.0) && ddlIn[1] >= static_cast<dataExt_t>(-1.0));
	// assert(ddlIn[0] < 8.0 && ddlIn[1] < 8.0);
	// assert(ddlIn[0] > -8.0 && ddlIn[1] > -8.0);

	/* ---------------------------------------------------------- */

//#ifndef __SYNTHESIS__
//	//TEMP PROBE it
//	static int ddlProbeCnt = 0;
//	ddlProbeCnt++;
//#endif

#ifndef __SYNTHESIS__
	ddlxnSampleCnt++;
	if (ddlxnChanCnt == MAXCHANNELS) ddlxnChanCnt = 0;
#endif


	channel_loop: for (int i = 0; i < MAXCHANNELS; i++) {
#pragma HLS UNROLL


#ifndef __SYNTHESIS__
		//TEMP PROBE
		ddlxnChanCnt++;
//		if (ddlxnSampleCnt >= PROBE_LOW && ddlxnSampleCnt <= PROBE_HIGH) {
//			std::cout << "xodDDLXN.h - Advance: sample[" << ddlxnSampleCnt
//					  << "],	channel = " << ddlxnChanCnt
//					  << "]: input = " << ddlIn[i]
//					  << std::endl;
//		}
#endif


		// TEMP - sort out ddl length init
		delayFractional[i] = ddlLenArray[i];		// delay length are initialized in constructor
		delayInteger[i] = static_cast<delayLenInt_t>(delayFractional[i]);
		delayFrac_t fracDelay = static_cast<delayFrac_t>(delayFractional[i] - delayInteger[i]);	 // calculate fractional difference

//		std::cout << "xodDDL5::advance delayFractional = " << static_cast<float>(delayFractional[i])
//				  << ", delayInteger = " << static_cast<float>(delayInteger[i])
//				  << ", fracDelay = " << static_cast<float>(fracDelay) << std::endl;


		if (readIndexP1[i] >= delayInteger[i] && writeIndex[i] >= delayInteger[i]) {
			// accounts for sudden changes in delayLength less than current pointer values
			writeIndex[i] = 0;
			readIndexP1[i] = 2;
		} else {
			if (readIndexP1[i] >= delayInteger[i]) {
				readIndexP1[i] = 0;
			}
			if (writeIndex[i] >= delayInteger[i]) {
				writeIndex[i] = 0;
			}
		}


//		if (readIndex[i] >= delayInteger[i] && writeIndex[i] >= delayInteger[i]) {
//			// accounts for sudden changes in delayLength less than current pointer values
//			writeIndex[i] = 0;
//			readIndex[i] = 1;
//		} else {
//			if (readIndex[i] >= delayInteger[i]) {
//				readIndex[i] = 0;
//			}
//			if (writeIndex[i] >= delayInteger[i]) {
//				writeIndex[i] = 0;
//			}
//		}
//		readIndexP1[i] = readIndex[i] + 1;
//		if (readIndexP1[i] >= delayInteger[i]) {
//			readIndexP1[i] = 0;
//		}
		//std::cout << "  write index = " << writeIndex << ", read index = " << readIndex  << ", read indexm1 = " << readIndexm1 << std::endl;


		//static fbShifter<dataExt_t, 1> dlyFeedback[MAXCHANNELS]; // appears that any length of shreg works, investigate consequences..

		// assume wetMix range = {0.0 : 1.0}
		delayCtrl_t dryMix = 1 - ddlCtrlAll.wetMix;
		// delayCtrl_t fbDryMix = static_cast<delayCtrl_t>(1 - ddlCtrlAll.feedbackMix);
		delayGain_t fbWetMix = static_cast<delayGain_t>(ddlCtrlAll.feedbackMix);	// scale fb gain

		//ddlRead[i] = ddlBuffer[i][readIndex[i]];
		ddlRead[i] = ddlPrevious[i];
		ddlReadP1[i] = ddlBuffer[i][readIndexP1[i]];
		ddlPrevious[i] = ddlReadP1[i];


		ddlInterp[i] = linInterpolate(ddlRead[i], ddlReadP1[i], fracDelay);	// linear interpolation function

		// *** Update Output ***
		ddlOut[i] = ddlCtrlAll.wetMix * ddlInterp[i] + dryMix * ddlIn[i];

		// CHECK output max values - (?)matches with top-level
		assert(ddlOut[i] >= -static_cast<dataExt_t>(2.0) && ddlOut[i] <= static_cast<dataExt_t>(2.0));

		// Unnecessary logic, test & REMOVE
//		if (delayInteger[i] == 0) {
//			ddlOut[i] = ddlCtrlAll.outputGain * ddlRead[i];
//
//		} else {
//			ddlInterp[i] = linInterpolate(ddlRead[i], ddlReadP1[i], fracDelay);	// linear interpolation function
//			ddlOut[i] = ddlCtrlAll.outputGain * ddlInterp[i];
//
//		}

		// internalFB[i] = dlyFeedback[i].shift(ddlOut[i]);
		internalFB[i] = dlyFeedback[i].shift(ddlInterp[i]);
		feedbackOut[i] = internalFB[i];


//#ifndef __SYNTHESIS__
//		//TEMP PROBE
//		if (ddlxnSampleCnt >= PROBE_LOW && ddlxnSampleCnt <= PROBE_HIGH) {
//			std::cout << "xodDDLXN.h - Advance: sample[" << ddlxnSampleCnt
//					  << "],	channel = " << ddlxnChanCnt
////				      << ",	ddlInterp[i] = " << static_cast<float>(ddlInterp[i])
////				      << ",	internalFB[i] = " << static_cast<float>(internalFB[i])
//					  << ",	output = " << static_cast<float>(ddlOut[i])
//					  << std::endl;
//		}
//#endif



		/* ---------------------------------------------------------- */

		if (!selectExtFB) {
			// double sum = input[c] + delayed[c]*decayGain;
			ddlMixed[i] = ddlIn[i] + fbWetMix * internalFB[i];
		} else {
			ddlMixed[i] = ddlIn[i] + fbWetMix * feedbackIn[i];
		}
		// Check for internal overflow
		//assert(ddlMixed[i] < static_cast<dataExt_t>(8.0) && ddlMixed[i] > static_cast<dataExt_t>(-8.0));
		assert(ddlMixed[i] < static_cast<dataExt_t>(8.0) && ddlMixed[i] > static_cast<dataExt_t>(-8.0));

//#ifndef __SYNTHESIS__
//		//TEMP PROBE it
//		if (ddlProbeCnt >= PROBE_LOW && ddlProbeCnt <= PROBE_HIGH) {
//			std::cout << "xodDDL5::advance[" << ddlProbeCnt << "]: ddlIn[" << i << "] = " << static_cast<float>(ddlIn[i])
//					  << ",  feedbackIn = " << static_cast<float>(feedbackIn[i]) << ",  feedbackGain = " << fbWetMix
//					  << ",  ddlMixed = " << static_cast<float>(ddlMixed[i])
//					  << std::endl;
//		}
//#endif


		/* ---------------------------------------------------------- */

		// write the input to the delay
		if (selectCross) {
			ddlBuffer[i][writeIndex[i]] = ddlMixed[(i+1) % MAXCHANNELS];    	//normalized feedback
		} else {
			ddlBuffer[i][writeIndex[i]] = ddlMixed[i];    						//normalized feedback
		}

		//readIndex[i]++;
		readIndexP1[i]++;
		writeIndex[i]++;

	}

	return;

}


/*------------------------------------------------------------------------------------------------*/

template<int maxChannels, int maxDelayFDN>
class DDLFDNFX {

	ap_uint<1> ddlCross = 0;
	ap_uint<1> extFB = 0;
	ddlCtrlAll_t ddlCtrl;
	ddlLenArray_t ddlLengthArr;
	ap_uint<1> houseMix = 0;
	ap_uint<1> ssbFbMod = 0;

	multiData_t feedbackIn {0};
	multiData_t feedbackMux {0};
	multiData_t feedbackOut {0};
	multiData_t houseIn {0};
	multiData_t houseOut {0};

	multiData_t ddlxnOut {0};

	// WARNING - analyze consequence of extended data type (overflows Zynq 25x18 Mpy)
	// ** isolated XodSSBMod analysis shows only slight resource increase for 32x18 filtering!!
	multiData_t ssbIn {0};
	multiData_t ssbOut {0};

	xodDDLXN<maxChannels, maxDelayFDN> ddlXNfdn;
	XodSSBMod<maxChannels, ddsDataWidth, ddsPhaseWidth, ddsPhaseScale, lutAddrWidth> ssbMod;


public:

	DDLFDNFX(ap_uint<1> selectCross, ap_uint<1> selectHouse, ap_uint<1> selectSSB,
			 ddlLenArray_t ddlLengthArray, ddlCtrlAll_t ddlCtrlAll) {

		ddlCross = selectCross;
		if (selectHouse or selectSSB) {
			extFB = 1;
		} else {
			extFB = 0;
		}
		houseMix = selectHouse;
		ssbFbMod = selectSSB;
		ddlCtrl.wetMix = ddlCtrlAll.wetMix;
		ddlCtrl.feedbackMix = ddlCtrlAll.feedbackMix;
		for (int c = 0; c < maxChannels; ++c) {
			ddlLengthArr[c] = ddlLengthArray[c];
			assert(ddlLengthArr[c] <= static_cast<delayLen_t>(MAXDELAY));
		}

	}


	void ddlFdnSet(ap_uint<1> selectCross, ap_uint<1> selectHouse, ap_uint<1> selectSSB,
				   ddlLenArray_t ddlLengthArray, ddlCtrlAll_t ddlCtrlAll) {
		ddlCross = selectCross;
		if (selectHouse or selectSSB) {
			extFB = 1;
		} else {
			extFB = 0;
		}
		houseMix = selectHouse;
		ssbFbMod = selectSSB;
		ddlCtrl.wetMix = ddlCtrlAll.wetMix;
		ddlCtrl.feedbackMix = ddlCtrlAll.feedbackMix;
		for (int c = 0; c < maxChannels; ++c) {
			ddlLengthArr[c] = ddlLengthArray[c];
			assert(ddlLengthArr[c] <= static_cast<delayLen_t>(MAXDELAY));
		}

	}


	void advance(ddsCtrl_t ssbModFreqCtrl[maxChannels], multiData_t ddlFdnIn, multiData_t &ddlFdnOut) {

		for (int c = 0; c < maxChannels; ++c) {
			assert(ddlFdnIn[c] <= static_cast<dataExt_t>(1.0) && ddlFdnIn[c] >= static_cast<dataExt_t>(-1.0));
		}

		ddlXNfdn.advance(ddlCross, extFB, ddlCtrl, ddlLengthArr, ddlFdnIn,
				         feedbackIn, feedbackOut, ddlxnOut);

		if (houseMix == 1) {
			// Mix using a Householder matrix
			//Householder<dataExt_t>::house(ddlOut, ddlMixed);
			//Householder<dataExt_t>::house(FB, FBMixed);

//#ifndef __SYNTHESIS__
//			// TEMP PROBE it
//			static int houseCnt = 0;
//			houseCnt++;
//#endif

			houseIn_loop: for (int c = 0; c < maxChannels; ++c) {
#pragma HLS UNROLL
				houseIn[c] = feedbackOut[c];

//#ifndef __SYNTHESIS__
//				// TEMP PROBE it
//				if (houseCnt >= PROBE_LOW && houseCnt <= PROBE_HIGH) {
//					std::cout << "xodReverbII.h[" << houseCnt << "]: houseIn[" << c << "] = " << static_cast<float>(houseIn[c]) << std::endl;
//				}
//#endif


			}

			// mix feedback delay network with householder function
			Householder<dataExt_t, static_cast<int>(maxChannels)>::house(houseIn, houseOut);

			houseMix_loop: for (int c = 0; c < maxChannels; ++c) {
#pragma HLS UNROLL
				feedbackMux[c] = houseOut[c];

//#ifndef __SYNTHESIS__
//				// TEMP PROBE it
//				if (houseCnt >= PROBE_LOW && houseCnt <= PROBE_HIGH) {
//					std::cout << "xodReverbII.h[" << houseCnt << "]: houseOut[" << c << "] = " << static_cast<float>(houseOut[c]) << std::endl;
//				}
//#endif

			}

		} else {
			feedback_loop: for (int c = 0; c < maxChannels; ++c) {
#pragma HLS UNROLL
				feedbackMux[c] = feedbackOut[c];
			}
		}


		ssbIn_loop: for (int c = 0; c < maxChannels; ++c) {
#pragma HLS UNROLL
			ssbIn[c] = feedbackMux[c];
		}

		ssbMod.advance(ssbModFreqCtrl, ssbIn, ssbOut);

		ssbFb_loop: for (int c = 0; c < maxChannels; ++c) {
#pragma HLS UNROLL
			if (ssbFbMod == 1) {
				feedbackIn[c] = ssbOut[c];
			} else {
				feedbackIn[c] = feedbackMux[c];
			}
		}

		output_loop: for (int c = 0; c < maxChannels; ++c) {
#pragma HLS UNROLL
			ddlFdnOut[c] = ddlxnOut[c];
		}
	}

};

/*------------------------------------------------------------------------------------------------*/


#endif

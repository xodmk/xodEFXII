/*------------------------------------------------------------------------------------------------*/
/* ___::((xodDDL5.cpp))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018:2022))::___
   ___::((created by eschei))___

	Purpose: Digital Delay C++
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

#include <cassert>
#include <ap_int.h>

#include "xodDDL5.h"


/* ---------------------------------------------------------- */

template <class T, class F>
T linInterpolate(T dataLow, T dataHigh, F fracDelay)
{
	T linInterpOut;
	linInterpOut = dataLow + (dataHigh - dataLow) * fracDelay;
	return linInterpOut;
}


//data_t linInterpolate(data_t dataLow, data_t dataHigh, delayFrac_t fracDelay)
//{
//	data_t linInterpOut;
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


/* ---------------------------------------------------------- */

void xodDDL5::advance(ap_uint<1> selectCross,
					  ap_uint<1> selectExtFB,
					  ddlCtrlAll_t ddlCtrlAll,
					  stereoData_t ddlIn,
					  stereoData_t feedbackIn,
					  stereoData_t &feedbackOut,
					  stereoData_t &ddlOut)
{
//#pragma HLS bind_storage variable=ddlBuffer type=RAM_2P impl=BRAM latency=1
//#pragma HLS DEPENDENCE variable=ddlBuffer inter false
////#pragma HLS ARRAY_PARTITION variable=ddlBuffer block factor=2 dim=1

#pragma HLS bind_storage variable=ddlBufferL type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=ddlBufferR type=RAM_T2P impl=BRAM latency=1
#pragma HLS ARRAY_PARTITION variable=ddlBufferL block factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=ddlBufferR block factor=2 dim=1
#pragma HLS DEPENDENCE variable=ddlBufferL inter false
#pragma HLS DEPENDENCE variable=ddlBufferR inter false
#pragma HLS DEPENDENCE variable=ddlBufferL intra false
#pragma HLS DEPENDENCE variable=ddlBufferR intra false

#pragma HLS PIPELINE

	assert(ddlIn.dataCH1 <= 1.0 && ddlIn.dataCH2 <= 1.0);
	assert(ddlIn.dataCH1 >= -1.0 && ddlIn.dataCH2 >= -1.0);


	//ddlIn.dataCH1 = 1.0;
	//ddlIn.dataCH2 = 1.0;

	data_t FBCH1tmp, FBCH2tmp = 0;

	delayFractional = ddlCtrlAll.delayLength;
	delayInteger = static_cast<delayInt_t>(delayFractional);
	delayFrac_t fracDelay = static_cast<delayFrac_t>(delayFractional - delayInteger);	 // calculate fractional difference

	//	std::cout << "xodDDL5::advance delayFractional = " << delayFractional << ", delayInteger = " << delayInteger
	//			  << ", fracDelay = " << fracDelay << std::endl;

	if (readIndex >= delayInteger && writeIndex >= delayInteger) {
		// accounts for sudden changes in delayLength less than current pointer values
		writeIndex = 0;
		readIndex = 1;
	} else {
		if (readIndex >= delayInteger) {
			readIndex = 0;
		}
		if (writeIndex >= delayInteger) {
			writeIndex = 0;
		}
	}
	readIndexm1 = readIndex + 1;
	if (readIndexm1 >= delayInteger) {
		readIndexm1 = 0;
	}
	//std::cout << "  write index = " << writeIndex << ", read index = " << readIndex  << ", read indexm1 = " << readIndexm1 << std::endl;


	// begin Digital Delay Line

    static fbShifter<data_t, 2> dlyFeedbackCH1; // appears that any length of shreg works, investigate consequences..
    static fbShifter<data_t, 2> dlyFeedbackCH2;

    // assume wetMix range = {0.0 : 1.0}
    delayCtrl_t dryMix = 1 - ddlCtrlAll.wetMix;
    delayCtrl_t fbDryMix = static_cast<delayCtrl_t>(1 - ddlCtrlAll.feedbackMix);
    delayGain_t fbWetMix = static_cast<delayGain_t>(ddlCtrlAll.feedbackMix);	// scale fb gain

//	ddlReadCH1 = ddlBuffer[readIndex].dataCH1;
//	ddlReadCH2 = ddlBuffer[readIndex].dataCH2;
//	ddlReadCH1m1 = ddlBuffer[readIndexm1].dataCH1;
//	ddlReadCH2m1 = ddlBuffer[readIndexm1].dataCH2;

	ddlReadCH1 = ddlBufferL[readIndex];
	ddlReadCH2 = ddlBufferR[readIndex];
	ddlReadCH1m1 = ddlBufferL[readIndexm1];
	ddlReadCH2m1 = ddlBufferR[readIndexm1];


	if (delayInteger == 0) {
		ddlOut.dataCH1 = dryMix * ddlIn.dataCH1 + ddlCtrlAll.wetMix * ddlReadCH1;
		ddlOut.dataCH2 = dryMix * ddlIn.dataCH2 + ddlCtrlAll.wetMix * ddlReadCH2;
		//ddlOut.dataCH1 = crossFader(ddlIn.dataCH1, ddlReadCH1, ddlCtrlAll.wetMix);
		//ddlOut.dataCH2 = crossFader(ddlIn.dataCH2, ddlReadCH2, ddlCtrlAll.wetMix);
	} else {
		//interpolate
		ddlInterpCH1 = linInterpolate(ddlReadCH1, ddlReadCH1m1, fracDelay);	// linear interpolation function
		ddlInterpCH2 = linInterpolate(ddlReadCH2, ddlReadCH2m1, fracDelay);	// linear interpolation function

		ddlOut.dataCH1 = ddlCtrlAll.outputGain * (dryMix * ddlIn.dataCH1 + ddlCtrlAll.wetMix * ddlInterpCH1);
		ddlOut.dataCH2 = ddlCtrlAll.outputGain * (dryMix * ddlIn.dataCH2 + ddlCtrlAll.wetMix * ddlInterpCH2);
		//ddlOut.dataCH1 = crossFader(ddlIn.dataCH1, ddlInterpCH1, ddlCtrlAll.wetMix);
		//ddlOut.dataCH2 = crossFader(ddlIn.dataCH2, ddlInterpCH2, ddlCtrlAll.wetMix);
	}

	assert(ddlOut.dataCH1 <= 1.0 && ddlOut.dataCH2 <= 1.0);
	assert(ddlOut.dataCH1 >= -1.0 && ddlOut.dataCH2 >= -1.0);

//	std::cout << "xodDDL5::advance ddlReadCH1 = " << ddlReadCH1 << ", ddlReadCH1m1 = " << ddlReadCH1m1
//			  << ", fracDelay = " << fracDelay << ", ddlInterpCH1 = " << ddlInterpCH1 << std::endl;

//	std::cout <<"xodDDL5::advance dryMix = " << dryMix << ", ddlIn.dataCH1 = " << ddlIn.dataCH1
//			  << ", ddlCtrlAll.wetMix = " << ddlCtrlAll.wetMix << ", ddlInterpCH1 = " << ddlInterpCH1
//			  << ", ddlpreOutGain = " << (dryMix * ddlIn.dataCH1 + ddlCtrlAll.wetMix * ddlInterpCH1)
//			  << ", ddlCtrlAll.outputGain = " << ddlCtrlAll.outputGain << std::endl;

	//std::cout<<"xodDDL5::advance ddlOut.dataCH1 = " << ddlOut.dataCH1 << ", ddlOut.dataCH2 = " << ddlOut.dataCH2 << std::endl;

	internalFB.dataCH1 = dlyFeedbackCH1.shift(ddlReadCH1);
	internalFB.dataCH2 = dlyFeedbackCH2.shift(ddlReadCH2);

	// feedbackOut = internalFB;
	feedbackOut.dataCH1 = internalFB.dataCH1;
	feedbackOut.dataCH2 = internalFB.dataCH2;

	if (!selectExtFB) {

		FBCH1tmp = fbDryMix * ddlIn.dataCH1 + fbWetMix * internalFB.dataCH1;
		FBCH2tmp = fbDryMix * ddlIn.dataCH2 + fbWetMix * internalFB.dataCH2;
		//FBCH1tmp = crossFader(ddlIn.dataCH1, internalFB.dataCH1, ddlCtrlAll.feedbackMix);
		//FBCH2tmp = crossFader(ddlIn.dataCH2, internalFB.dataCH2, ddlCtrlAll.feedbackMix);

		assert(FBCH1tmp <= 1.0 && FBCH2tmp <= 1.0);
		assert(FBCH1tmp >= -1.0 && FBCH2tmp >= -1.0);

		FBCH1 = (FBCH1tmp > static_cast<data_t>(1.0)) ? static_cast<data_t>(1.0) : FBCH1tmp;
		FBCH1 = (FBCH1tmp < static_cast<data_t>(-1.0)) ? static_cast<data_t>(-1.0) : FBCH1tmp;
		FBCH2 = (FBCH2tmp > static_cast<data_t>(1.0)) ? static_cast<data_t>(1.0) : FBCH2tmp;
		FBCH2 = (FBCH2tmp < static_cast<data_t>(-1.0)) ? static_cast<data_t>(-1.0) : FBCH2tmp;

	} else {

		FBCH1tmp = fbDryMix * ddlIn.dataCH1 + fbWetMix * feedbackIn.dataCH1;
		FBCH2tmp = fbDryMix * ddlIn.dataCH2 + fbWetMix * feedbackIn.dataCH2;
		//FBCH1 = crossFader(ddlIn.dataCH1, feedbackIn.dataCH1, ddlCtrlAll.feedbackMix);
		//FBCH2 = crossFader(ddlIn.dataCH2, feedbackIn.dataCH2, ddlCtrlAll.feedbackMix);

		assert(FBCH1tmp <= 1.0 && FBCH2tmp <= 1.0);
		assert(FBCH1tmp >= -1.0 && FBCH2tmp >= -1.0);

		FBCH1 = (FBCH1tmp > static_cast<data_t>(1.0)) ? static_cast<data_t>(1.0) : FBCH1tmp;
		FBCH1 = (FBCH1tmp < static_cast<data_t>(-1.0)) ? static_cast<data_t>(-1.0) : FBCH1tmp;
		FBCH2 = (FBCH2tmp > static_cast<data_t>(1.0)) ? static_cast<data_t>(1.0) : FBCH2tmp;
		FBCH2 = (FBCH2tmp < static_cast<data_t>(-1.0)) ? static_cast<data_t>(-1.0) : FBCH2tmp;

	}

//	std::cout << "xodDDL5::advance ddlIn = " << ddlIn.dataCH1 << ",  fbDryMix = " << fbDryMix
//			  << ",  feedbackIn = " << feedbackIn.dataCH1 << ",  feedbackGain = " << fbWetMix
//			  << ",  dryMix = " << fbDryMix * ddlIn.dataCH1 << ",  feedbackMix = " << fbWetMix * feedbackIn.dataCH1
//			  << ",  FBCH1 = " << FBCH1 << ",  feedbackOut = " << feedbackOut.dataCH1 << std::endl;


	// write the input to the delay

//	if (selectCross) {
//		ddlBuffer[writeIndex].dataCH2 = FBCH1;    	//normalized feedback
//		ddlBuffer[writeIndex].dataCH1 = FBCH2;    	//normalized feedback
//	} else {
//		ddlBuffer[writeIndex].dataCH1 = FBCH1;    	//normalized feedback
//		ddlBuffer[writeIndex].dataCH2 = FBCH2;    	//normalized feedback
//	}

	if (selectCross) {
		ddlBufferL[writeIndex] = FBCH2;    	//normalized feedback
		ddlBufferR[writeIndex] = FBCH1;    	//normalized feedback
	} else {
		ddlBufferL[writeIndex] = FBCH1;    	//normalized feedback
		ddlBufferR[writeIndex] = FBCH2;    	//normalized feedback
	}

	readIndex++;
	writeIndex++;



	//std::cout << "xodDDL5::advance ddlOut = " << static_cast<float>(ddlOut.dataCH1) << ", "
	//          << static_cast<float>(ddlOut.dataCH2) << std::endl;

	return;

}

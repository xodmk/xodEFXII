/*------------------------------------------------------------------------------------------------*/
/* ___::((xodDDL5.h))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018))::___
   ___::((created by eschei))___

	Purpose: header .h for Digital Delay
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

#include <math.h>
#include <ap_int.h>
#include "ap_shift_reg.h"

#include "xodEFX.h"
#include "xodCrossFader.h"


/*------------------------------------------------------------------------------------------------*/
// *--- DDL Types

//#define MAXDELAY 24000
//
////const unsigned DLYBWIDTH = ceil(log2(MAXDELAY));
//const unsigned DLYBWIDTH = 16;
//const unsigned DLYFRACWIDTH = 8;
//
//typedef ap_ufixed<16, 1> delayCtrl_t;
//typedef ap_uint<DLYBWIDTH> delayInt_t;
//typedef ap_ufixed<DLYBWIDTH+DLYFRACWIDTH, DLYBWIDTH> delay_t;
//typedef ap_ufixed<DLYFRACWIDTH, 0> delayFrac_t;


//struct ddlCtrlAll_t {
//	delay_t delayLength;
//	delayCtrl_t wetMix;
//	delayCtrl_t feedbackMix;
//	delayCtrl_t outputGain;
//};

/*------------------------------------------------------------------------------------------------*/

class xodDDL5 {

	//CrossFader<data_t, lutAddrWidth> xFader;

	// internal variables
	//stereoData_t ddlBuffer[MAXDELAY];
	data_t ddlBufferL[MAXDELAY];
	data_t ddlBufferR[MAXDELAY];
	delayInt_t readIndex;
	delayInt_t readIndexm1;
	delayInt_t writeIndex;
	stereoData_t internalFB;

	delayInt_t delayInteger;
	delay_t delayFractional;

	data_t ddlReadCH1, ddlReadCH2;
	data_t ddlReadCH1m1, ddlReadCH2m1;
	data_t ddlInterpCH1, ddlInterpCH2;
	data_t ddlWetCH1, ddlWetCH2;
	data_t FBCH1, FBCH2;


public:

	//ap_uint<1> selectCross;
	//ap_uint<1> useExternalFB;	// 0 = normalized internal FB; 1 = external FB
	//delayCtrl_t wetMix;
	//delayCtrl_t feedbackMix;
	//delayCtrl_t outputGain;

	xodDDL5()
	{
		//newDlengthFlag = 0;
		//selectCross = 0;
		//useExternalFB = 0;
		//wetMix = 0.5;
		//feedbackMix = 0.0;
		//outputGain = 1.0;

		delayInteger = 5600;
		delayFractional = 5600.5;

		readIndex = 1;
		writeIndex = 0;
	}

	void advance(ap_uint<1> selectCross,
				 ap_uint<1> selectExtFB,
				 ddlCtrlAll_t ddlCtrlAll,
				 stereoData_t ddlIn,
				 stereoData_t feedbackIn,
				 stereoData_t &feedbackOut,
				 stereoData_t &ddlOut);

};

#endif

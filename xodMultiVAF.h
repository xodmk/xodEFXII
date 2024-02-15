/*------------------------------------------------------------------------------------------------*/
/* ___::((xodMultiVAF.hpp))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018:2022))::___
   ___::((created by eschei))___

	Purpose: Reference model cpp for XODMK VA Filter (Stereo Version)
		 	 * Multi TPT 1-pole filter * Moog Half Ladder 2pole * Moog Ladder 4pole filter *
	         HLS implementation of Virtual Analog Filters for FPGA
			 IIR Toplogy-Preserving Transform (TPT) Filters
			 "The Art Of VA Filter Design" - Vadim Zavalishin

	Device: All, Zync SoC
	Revision History: Feb 14, 2018 	- initial
	Revision History: Mar 1, 2018 	- working beta
	Revision History: 2023-08-24   	- Modified code to match xodVAFilterMono design & Fixed tb
*/


/*------------------------------------------------------------------------------------------------*/
/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/
/*------------------------------------------------------------------------------------------------*/

#include <math.h>
#include <ap_int.h>
#include "hls_stream.h"

#include "xodEFXII_types.h"


#ifndef __XODMULTIVAF_H__
#define __XODMULTIVAF_H__



// *-------------------------------------------------------------------------------------------* //
// *--- 1-pole TPT Low-Pass Filter HLS ---* //

//class onePoleTPT_LP {
//public:
//
//protected:
//	// controls
//	ctrl_t G;				// Pre-Warped Cutoff (G calculated by Processor)
//	stereoData_t z1;		// z-1 register
//
//public:
//	inline void initialize_LP(ctrl_t newG) {
//		G = newG;
//		z1.dataL = 0;
//		z1.dataR = 0;
//	}
//
//	void setG_LP(ctrl_t newG);
//	void doFilterStage_LP(stereoData_t xn,  stereoData_t& z1fb, stereoData_t& ynLP);
//};


// *-------------------------------------------------------------------------------------------* //
// *--- 1-pole TPT All-Pass Filter HLS ---* //

//class onePoleTPT_AP {
//public:
//
//protected:
//	// controls
//	ctrl_t G;				// cutoff
//	stereoData_t z1;		// z-1 register
//
//public:
//	inline void initialize_AP(ctrl_t newG) {
//		G = newG;
//		z1.dataL = 0;
//		z1.dataR = 0;
//	}
//
//	void setG_AP(ctrl_t newG);
//	void doFilterStage_AP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynAP);
//};



// *-------------------------------------------------------------------------------------------* //
// *--- Stereo Moog Half Ladder Filter HLS ---* //

class onePoleTPT_LPHL1 {
public:

protected:
	// controls
	fltCtrl_t G;					// Pre-Warped Cutoff (G calculated by Processor)
	stereoData_t z1;			// z-1 register

public:
	inline void initialize_LP(fltCtrl_t newG) {
		G = newG;
		z1.dataCH1 = 0;
		z1.dataCH2 = 0;
	}
	void setG_LP(fltCtrl_t newG);
	void doFilterStage_LP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynLP);
};


class onePoleTPT_LPHL2 {
public:

protected:
	// controls
	fltCtrl_t G;					// Pre-Warped Cutoff (G calculated by Processor)
	stereoData_t z1;			// z-1 register

public:
	inline void initialize_LP(fltCtrl_t newG) {
		G = newG;
		z1.dataCH1 = 0;
		z1.dataCH2 = 0;
	}
	void setG_LP(fltCtrl_t newG);
	void doFilterStage_LP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynLP);
};


class onePoleTPT_APHL1 {
public:

protected:
	// controls
	fltCtrl_t G;				// cutoff
	stereoData_t z1;		// z-1 register

public:
	inline void initialize_AP(fltCtrl_t newG) {
		G = newG;
		z1.dataCH1 = 0;
		z1.dataCH2 = 0;
	}

	void setG_AP(fltCtrl_t newG);
	void doFilterStage_AP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynAP);
};


// *--------------------------------------------------------* //

class xodMoogHalfLadder {
public:

	onePoleTPT_LPHL1 LPHL1;
	onePoleTPT_LPHL2 LPHL2;
	onePoleTPT_APHL1 APHL1;

protected:
	// controls
	fltCtrl_t G;				// Pre-Warped Cutoff (G calculated by Processor)

	stereoData_t SM;

	stereoData_t fBeta1;
	stereoData_t fBeta2;
	stereoData_t fBeta3;

	stereoData_t ynStage1;
	stereoData_t ynStage2;

	stereoData_t z1fb_1;
	stereoData_t z1fb_2;
	stereoData_t z1fb_3;


public:
	void initialize(fltCtrl_t newG);
	void setFcAndRes_uMHL(float cutoff, float resonance, float sampleRate);
	void advance(fltCtrl_t K , fltCtrl_t G, fParam_t fParam, stereoData_t xn, stereoData_t& yn);
};


// *-------------------------------------------------------------------------------------------* //
// *--- mono Moog Ladder 4-Pole Filter HLS ---* //

class onePoleTPT_LPML1 {
public:

protected:
	// controls
	fltCtrl_t G;				// Pre-Warped Cutoff (G calculated by Processor)
	stereoData_t z1;		// z-1 register

public:
	inline void initialize_LP(fltCtrl_t newG) {
		G = newG;
		z1.dataCH1 = 0;
		z1.dataCH2 = 0;
	}

	void setG_LP(fltCtrl_t newG);
	void doFilterStage_LP(stereoData_t xn,  stereoData_t& z1fb, stereoData_t& ynLP);
};


class onePoleTPT_LPML2 {
public:

protected:
	// controls
	fltCtrl_t G;				// Pre-Warped Cutoff (G calculated by Processor)
	stereoData_t z1;		// z-1 register

public:
	inline void initialize_LP(fltCtrl_t newG) {
		G = newG;
		z1.dataCH1 = 0;
		z1.dataCH2 = 0;
	}

	void setG_LP(fltCtrl_t newG);
	void doFilterStage_LP(stereoData_t xn,  stereoData_t& z1fb, stereoData_t& ynLP);
};


class onePoleTPT_LPML3 {
public:

protected:
	// controls
	fltCtrl_t G;				// Pre-Warped Cutoff (G calculated by Processor)
	stereoData_t z1;			// z-1 register

public:
	inline void initialize_LP(fltCtrl_t newG) {
		G = newG;
		z1.dataCH1 = 0;
		z1.dataCH2 = 0;
	}

	void setG_LP(fltCtrl_t newG);
	void doFilterStage_LP(stereoData_t xn,  stereoData_t& z1fb, stereoData_t& ynLP);
};


class onePoleTPT_LPML4 {
public:

protected:
	// controls
	fltCtrl_t G;				// Pre-Warped Cutoff (G calculated by Processor)
	stereoData_t z1;			// z-1 register

public:
	inline void initialize_LP(fltCtrl_t newG) {
		G = newG;
		z1.dataCH1 = 0;
		z1.dataCH2 = 0;
	}

	void setG_LP(fltCtrl_t newG);
	void doFilterStage_LP(stereoData_t xn,  stereoData_t& z1fb, stereoData_t& ynLP);
};

// *--------------------------------------------------------* //


class moogLadder4p {
public:

	onePoleTPT_LPML1 LPF1;
	onePoleTPT_LPML2 LPF2;
	onePoleTPT_LPML3 LPF3;
	onePoleTPT_LPML4 LPF4;


protected:
	// controls
	fltCtrl_t G;

	moogL_t moogL4p;

	stereoData_t SM;

	stereoData_t ynStage1;
	stereoData_t ynStage2;
	stereoData_t ynStage3;

	stereoData_t fb1;
	stereoData_t fb2;
	stereoData_t fb3;
	stereoData_t fb4;


public:
	void initialize(fltCtrl_t newG);
	void doFilters(fltCtrl_t G);
	void advance(fltCtrl_t K , fltCtrl_t G, fParam_t fParam, stereoData_t xn, stereoData_t& yn);
};


// *-------------------------------------------------------------------------------------------* //
// *--- Stereo TPT SEM SVF Multi-mode filter ---* //

class SEMSVF {
public:
	// controls
	fltCtrl_t alpha0;
	fltCtrl_t alpha;
	fltCtrl_t rho;

protected:


	stereoData_t z1_LPF1;		// LP1 feedback
	stereoData_t z1_LPF2;		// LP2 feedback

	stereoData_t ynStage0;		// HP output
	stereoData_t ynStage1;		// BP output
	stereoData_t ynStage2;		// LP output


public:
	inline void initialize(void) {

		z1_LPF1.dataCH1 = 0;
		z1_LPF1.dataCH2 = 0;
		z1_LPF2.dataCH1 = 0;
		z1_LPF2.dataCH2 = 0;

		ynStage0.dataCH1 = 0;
		ynStage0.dataCH2 = 0;
		ynStage1.dataCH1 = 0;
		ynStage1.dataCH2 = 0;
		ynStage2.dataCH1 = 0;
		ynStage2.dataCH2 = 0;

	}

	void update(fltCtrl_t newAlpha0, fltCtrl_t newAlpha, fltCtrl_t newRho);
	void advance(filterType_t fType, stereoData_t xn, stereoData_t& yn);
};


// *-------------------------------------------------------------------------------------------* //

extern void xodMultiVAF_top(ap_uint<1> rst, fltCtrl_t K, fltCtrl_t G, fParam_t fParam,
		                    stereoData_t xn, stereoData_t& yn);


// *-------------------------------------------------------------------------------------------* //

#endif // __XODMULTIVAF_H__

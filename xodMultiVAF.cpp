/*------------------------------------------------------------------------------------------------*/
/* ___::((xodMultiVAF.cpp))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018:2023))::___
   ___::((created by eschei))___

	Purpose: Definition cpp for XODMK Moog Ladder 4pole filter
	         HLS implementation of Virtual Analog Filters for FPGA
			 IIR Toplogy-Preserving Transform (TPT) Filters
			 "The Art Of VA Filter Design" - Vadim Zavalishin

	Device: All, Zync SoC
	Revision History: Feb 14, 2018 - initial
	Revision History: Mar 1, 2018 - working beta
	Revision History: 2022-05-19   - XOD AudioFile tb update
*/

/*------------------------------------------------------------------------------------------------*/
/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/
/*------------------------------------------------------------------------------------------------*/

#include <math.h>
#include <ap_int.h>
#include <cassert>

//#include "xodMultiVAF_types.h"
#include "xodEFXII_types.h"
#include "xodMultiVAF.h"


// *--------------------------------------------------------* //
// *--- fbShifter ---* //

//// **warning** note that we can't put delays that are smaller than latencies of
//// the generated HW RTL and those have to be found empirically (?)...
//template <typename T, unsigned int fbDelay>
//class fbShifter {
//    unsigned int fbPtr;
//    T feedback[fbDelay];
//public:
//    fbShifter() : fbPtr(0) {}
//
//    T shift(T shiftData) {
//        #pragma HLS dependence variable=feedback false
//        T t = feedback[fbPtr];
//        feedback[fbPtr] = shiftData;
//        fbPtr = fbPtr==fbDelay-1? 0 : fbPtr+1; assert(fbPtr < fbDelay);
//        return t;
//    }
//};


// *-------------------------------------------------------------------------------------------* //
// *--- 1-pole TPT Low-Pass Filter HLS ---* //

//// set Pre-Warped cutoff 'G' parameter
//void onePoleTPT_LP::setG_LP(ctrl_t newG) {
//	G = newG;
//}
//
//
//// TPT 1-pole filter - Zavalishin p46 (the Art of VA Design)
//void onePoleTPT_LP::doFilterStage_LP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynLP)
//{
//	stereoData_t v;
//	v.dataL 	= (xn.dataL - z1.dataL) * G;
//	v.dataR 	= (xn.dataR - z1.dataR) * G;
//	ynLP.dataL 	= v.dataL + z1.dataL;
//	ynLP.dataR 	= v.dataR + z1.dataR;
//    z1fb.dataL 	= z1.dataL;
//    z1fb.dataR 	= z1.dataR;
//
////#ifndef __SYNTHESIS__
////	//TEMP PROBE it
////	static int efxDataCnt = 0;
////	efxDataCnt++;
////	if (efxDataCnt >= PROBE_LOW && efxDataCnt <= PROBE_HIGH) {
////		std::cout << "onePoleTPT_LP: G = " << G.to_float() << ",	xn.L = " << xn.dataL.to_float() << ",	v.L = " << v.dataL.to_float()
////				  << ",	ynLP.L = " << ynLP.dataL.to_float() << ",	z1.L = " << z1.dataL.to_float() << std::endl;
////	}
////#endif
//
//	z1.dataL = ynLP.dataL + v.dataL;
//	z1.dataR = ynLP.dataR + v.dataR;
//}
//
//
//// *-------------------------------------------------------------------------------------------* //
//// *--- 1-pole TPT All-Pass Filter HLS ---* //
//
//void onePoleTPT_AP::setG_AP(ctrl_t newG) {
//	G = newG;
//}
//
//// TPT 1-pole filter - Zavalishin p46 (the Art of VA Design)
//void onePoleTPT_AP::doFilterStage_AP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynAP)
//{
//	stereoData_t v;
//	stereoData_t LP;
//	stereoData_t HP;
//
//	v.dataL 	= (xn.dataL - z1.dataL) * G;
//	v.dataR 	= (xn.dataR - z1.dataR) * G;
//
//	LP.dataL 	= v.dataL + z1.dataL;
//	LP.dataR 	= v.dataR + z1.dataR;
//	HP.dataL 	= xn.dataL - LP.dataL;
//	HP.dataR 	= xn.dataR - LP.dataR;
//
//	ynAP.dataL 	= LP.dataL - HP.dataL;
//	ynAP.dataR 	= LP.dataR - HP.dataR;
//    z1fb.dataL 	= z1.dataL;
//    z1fb.dataR 	= z1.dataR;
//
//	z1.dataL 	= LP.dataL + v.dataL;
//	z1.dataR 	= LP.dataR + v.dataR;
//}


// *-------------------------------------------------------------------------------------------* //
// *--- Stereo Moog Half Ladder Filter HLS ---* //

void onePoleTPT_LPHL1::setG_LP(fltCtrl_t newG) {
	G = newG;
}

// TPT 1-pole filter - Zavalishin p46 (the Art of VA Design)
void onePoleTPT_LPHL1::doFilterStage_LP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynLP)
{
	stereoData_t v;
	v.dataCH1 		= (xn.dataCH1 - z1.dataCH1) * G;
	v.dataCH2 		= (xn.dataCH2 - z1.dataCH2) * G;
	ynLP.dataCH1 	= v.dataCH1 + z1.dataCH1;
	ynLP.dataCH2 	= v.dataCH2 + z1.dataCH2;
	z1.dataCH1 		= ynLP.dataCH1 + v.dataCH1;
	z1.dataCH2 		= ynLP.dataCH2 + v.dataCH2;
	z1fb.dataCH1 	= z1.dataCH1;
	z1fb.dataCH2 	= z1.dataCH2;
}


void onePoleTPT_LPHL2::setG_LP(fltCtrl_t newG) {
	G = newG;
}

// TPT 1-pole filter - Zavalishin p46 (the Art of VA Design)
void onePoleTPT_LPHL2::doFilterStage_LP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynLP)
{
	stereoData_t v;
	v.dataCH1 		= (xn.dataCH1 - z1.dataCH1) * G;
	v.dataCH2 		= (xn.dataCH2 - z1.dataCH2) * G;
	ynLP.dataCH1 	= v.dataCH1 + z1.dataCH1;
	ynLP.dataCH2 	= v.dataCH2 + z1.dataCH2;
	z1.dataCH1 		= ynLP.dataCH1 + v.dataCH1;
	z1.dataCH2 		= ynLP.dataCH2 + v.dataCH2;
	z1fb.dataCH1 	= z1.dataCH1;
	z1fb.dataCH2 	= z1.dataCH2;
}


void onePoleTPT_APHL1::setG_AP(fltCtrl_t newG) {
	G = newG;
}

// TPT 1-pole filter - Zavalishin p46 (the Art of VA Design)
void onePoleTPT_APHL1::doFilterStage_AP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynAP)
{
	stereoData_t v;
	stereoData_t LP;
	stereoData_t HP;

	v.dataCH1 		= (xn.dataCH1 - z1.dataCH1) * G;
	v.dataCH2 		= (xn.dataCH2 - z1.dataCH2) * G;

	LP.dataCH1 		= v.dataCH1 + z1.dataCH1;
	LP.dataCH2 		= v.dataCH2 + z1.dataCH2;
	HP.dataCH1 		= xn.dataCH1 - LP.dataCH1;
	HP.dataCH2 		= xn.dataCH2 - LP.dataCH2;

	ynAP.dataCH1 	= LP.dataCH1 - HP.dataCH1;
	ynAP.dataCH2 	= LP.dataCH2 - HP.dataCH2;
    z1fb.dataCH1 	= z1.dataCH1;
    z1fb.dataCH2 	= z1.dataCH2;

	z1.dataCH1 		= LP.dataCH1 + v.dataCH1;
	z1.dataCH2 		= LP.dataCH2 + v.dataCH2;
}


void xodMoogHalfLadder::initialize(fltCtrl_t newG)
{
	G = newG;

	SM.dataCH1 = 0;
	SM.dataCH2 = 0;

	ynStage1.dataCH1 = 0;
	ynStage1.dataCH2 = 0;
	ynStage2.dataCH1 = 0;
	ynStage2.dataCH2 = 0;

	z1fb_1.dataCH1 = 0;
	z1fb_1.dataCH2 = 0;
	z1fb_2.dataCH1 = 0;
	z1fb_2.dataCH2 = 0;
	z1fb_3.dataCH1 = 0;
	z1fb_3.dataCH2 = 0;

	LPHL1.initialize_LP(G);
	LPHL2.initialize_LP(G);
	APHL1.initialize_AP(G);
}

void xodMoogHalfLadder::advance(fltCtrl_t K, fltCtrl_t G, fParam_t fParam, stereoData_t xn, stereoData_t& yn)
{
	// sum 3 internal Z1 states
	SM.dataCH1 = fParam.beta1 * z1fb_1.dataCH1 + fParam.beta2 * z1fb_2.dataCH1 + fParam.beta3 * z1fb_3.dataCH1;
	SM.dataCH2 = fParam.beta1 * z1fb_1.dataCH2 + fParam.beta2 * z1fb_2.dataCH2 + fParam.beta3 * z1fb_3.dataCH2;

	stereoData_t un;
	un.dataCH1 = fParam.alpha0 * (xn.dataCH1 - K * SM.dataCH1);
	un.dataCH2 = fParam.alpha0 * (xn.dataCH2 - K * SM.dataCH2);

	LPHL1.doFilterStage_LP(un, z1fb_1, ynStage1);
	LPHL2.doFilterStage_LP(ynStage1, z1fb_2, ynStage2);
	APHL1.doFilterStage_AP(ynStage2, z1fb_3, yn);
}



// *-------------------------------------------------------------------------------------------* //
// *--- Stereo Moog Ladder 4-Pole Filter HLS ---* //


void onePoleTPT_LPML1::setG_LP(fltCtrl_t newG) {
	G = newG;
}


// TPT 1-pole filter - Zavalishin p46 (the Art of VA Design)
void onePoleTPT_LPML1::doFilterStage_LP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynLP)
{
	stereoData_t v;
	v.dataCH1 		= (xn.dataCH1 - z1.dataCH1) * G;
	v.dataCH2 		= (xn.dataCH2 - z1.dataCH2) * G;
	ynLP.dataCH1 	= v.dataCH1 + z1.dataCH1;
	ynLP.dataCH2 	= v.dataCH2 + z1.dataCH2;
    z1fb.dataCH1 	= z1.dataCH1;
    z1fb.dataCH2 	= z1.dataCH2;
	z1.dataCH1 		= ynLP.dataCH1 + v.dataCH1;
	z1.dataCH2 		= ynLP.dataCH2 + v.dataCH2;
}


void onePoleTPT_LPML2::setG_LP(fltCtrl_t newG) {
	G = newG;
}

// TPT 1-pole filter - Zavalishin p46 (the Art of VA Design)
void onePoleTPT_LPML2::doFilterStage_LP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynLP)
{
	stereoData_t v;
	v.dataCH1 		= (xn.dataCH1 - z1.dataCH1) * G;
	v.dataCH2 		= (xn.dataCH2 - z1.dataCH2) * G;
	ynLP.dataCH1 	= v.dataCH1 + z1.dataCH1;
	ynLP.dataCH2 	= v.dataCH2 + z1.dataCH2;
    z1fb.dataCH1 	= z1.dataCH1;
    z1fb.dataCH2 	= z1.dataCH2;
	z1.dataCH1 		= ynLP.dataCH1 + v.dataCH1;
	z1.dataCH2 		= ynLP.dataCH2 + v.dataCH2;
}


void onePoleTPT_LPML3::setG_LP(fltCtrl_t newG) {
	G = newG;
}

// TPT 1-pole filter - Zavalishin p46 (the Art of VA Design)
void onePoleTPT_LPML3::doFilterStage_LP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynLP)
{
	stereoData_t v;
	v.dataCH1 		= (xn.dataCH1 - z1.dataCH1) * G;
	v.dataCH2 		= (xn.dataCH2 - z1.dataCH2) * G;
	ynLP.dataCH1 	= v.dataCH1 + z1.dataCH1;
	ynLP.dataCH2 	= v.dataCH2 + z1.dataCH2;
    z1fb.dataCH1 	= z1.dataCH1;
    z1fb.dataCH2 	= z1.dataCH2;
	z1.dataCH1 		= ynLP.dataCH1 + v.dataCH1;
	z1.dataCH2 		= ynLP.dataCH2 + v.dataCH2;
}


void onePoleTPT_LPML4::setG_LP(fltCtrl_t newG) {
	G = newG;
}

// TPT 1-pole filter - Zavalishin p46 (the Art of VA Design)
void onePoleTPT_LPML4::doFilterStage_LP(stereoData_t xn, stereoData_t& z1fb, stereoData_t& ynLP)
{
	stereoData_t v;
	v.dataCH1    	= (xn.dataCH1 - z1.dataCH1) * G;
	v.dataCH2    	= (xn.dataCH2 - z1.dataCH2) * G;
	ynLP.dataCH1 	= v.dataCH1 + z1.dataCH1;
	ynLP.dataCH2 	= v.dataCH2 + z1.dataCH2;
    z1fb.dataCH1 	= z1.dataCH1;
    z1fb.dataCH2 	= z1.dataCH2;
	z1.dataCH1   	= ynLP.dataCH1 + v.dataCH1;
	z1.dataCH2   	= ynLP.dataCH2 + v.dataCH2;
}

// *--------------------------------------------------------* //

void moogLadder4p::initialize(fltCtrl_t newG)
{
	G = newG;

	moogL4p.xn.dataCH1 = 0;
	moogL4p.xn.dataCH2 = 0;
	moogL4p.yn.dataCH1 = 0;
	moogL4p.yn.dataCH2 = 0;

	moogL4p.fb_LP1.dataCH1 = 0;
	moogL4p.fb_LP1.dataCH2 = 0;
	moogL4p.fb_LP2.dataCH1 = 0;
	moogL4p.fb_LP2.dataCH2 = 0;
	moogL4p.fb_LP3.dataCH1 = 0;
	moogL4p.fb_LP3.dataCH2 = 0;
	moogL4p.fb_LP4.dataCH1 = 0;
	moogL4p.fb_LP4.dataCH2 = 0;

	SM.dataCH1 = 0;
	SM.dataCH2 = 0;

	ynStage1.dataCH1 = 0;
	ynStage1.dataCH2 = 0;
	ynStage2.dataCH1 = 0;
	ynStage2.dataCH2 = 0;
	ynStage3.dataCH1 = 0;
	ynStage3.dataCH2 = 0;

	fb1.dataCH1 = 0;
	fb1.dataCH2 = 0;
	fb2.dataCH1 = 0;
	fb2.dataCH2 = 0;
	fb3.dataCH1 = 0;
	fb3.dataCH2 = 0;
	fb4.dataCH1 = 0;
	fb4.dataCH2 = 0;

	LPF1.initialize_LP(G);
	LPF2.initialize_LP(G);
	LPF3.initialize_LP(G);
	LPF4.initialize_LP(G);
}


// *--------------------------------------------------------* //

void moogLadder4p::doFilters(fltCtrl_t G)
{
#pragma dataflow

	LPF1.setG_LP(G);
	LPF2.setG_LP(G);
	LPF3.setG_LP(G);
	LPF4.setG_LP(G);

	LPF1.doFilterStage_LP(moogL4p.xn, moogL4p.fb_LP1, ynStage1);
	LPF2.doFilterStage_LP(ynStage1, moogL4p.fb_LP2, ynStage2);
	LPF3.doFilterStage_LP(ynStage2, moogL4p.fb_LP3, ynStage3);
	LPF4.doFilterStage_LP(ynStage3, moogL4p.fb_LP4, moogL4p.yn);
}


void moogLadder4p::advance(fltCtrl_t K, fltCtrl_t G, fParam_t fParam, stereoData_t xn, stereoData_t& yn)
{
#pragma HLS PIPELINE

	// sum 3 internal Z1 states
	SM.dataCH1 = fParam.beta1 * fb1.dataCH1 + fParam.beta2 * fb2.dataCH1 +
			   fParam.beta3 * fb3.dataCH1 + fParam.beta4 * fb4.dataCH1;
	SM.dataCH2 = fParam.beta1 * fb1.dataCH2 + fParam.beta2 * fb2.dataCH2 +
			   fParam.beta3 * fb3.dataCH2 + fParam.beta4 * fb4.dataCH2;

	stereoData_t un;
	un.dataCH1 = fParam.alpha0 * (xn.dataCH1 - K * SM.dataCH1);
	un.dataCH2 = fParam.alpha0 * (xn.dataCH2 - K * SM.dataCH2);

	moogL4p.xn.dataCH1 = un.dataCH1;
	moogL4p.xn.dataCH2 = un.dataCH2;

	//std::cout<<"xnL = " << xn << ",	fb1 = " << fb1 << ",	fb2 = " << fb2
	//                    << ",	fb3 = " << fb3 << ",	fAlpha0 = " << fParam.alpha0
	//					  << ",	SM = " << SM << ",	un = " << un << std::endl;

	doFilters(G);

	yn.dataCH1 	= moogL4p.yn.dataCH1;
	yn.dataCH2 	= moogL4p.yn.dataCH2;

	fb1.dataCH1 	= moogL4p.fb_LP1.dataCH1;
	fb1.dataCH2 	= moogL4p.fb_LP1.dataCH2;
	fb2.dataCH1 	= moogL4p.fb_LP2.dataCH1;
	fb2.dataCH2 	= moogL4p.fb_LP2.dataCH2;
	fb3.dataCH1 	= moogL4p.fb_LP3.dataCH1;
	fb3.dataCH2 	= moogL4p.fb_LP3.dataCH2;
	fb4.dataCH1 	= moogL4p.fb_LP4.dataCH1;
	fb4.dataCH2 	= moogL4p.fb_LP4.dataCH2;
}


// *-------------------------------------------------------------------------------------------* //
// *--- Stereo TPT SEM SVF Multi-mode filter ---* //

void SEMSVF::update(fltCtrl_t newAlpha0, fltCtrl_t newAlpha, fltCtrl_t newRho)
{
	alpha0 = newAlpha0;
	alpha  = newAlpha;
	rho    = newRho;
}


void SEMSVF::advance(filterType_t fType, stereoData_t xn, stereoData_t& yn)
{
	stereoData_t ynLP {0};
	stereoData_t ynHP {0};
	stereoData_t ynBP {0};
	stereoData_t ynBS {0};

	// form the HP output first
	ynStage0.dataCH1 = alpha0 * (xn.dataCH1 - rho * z1_LPF1.dataCH1 - z1_LPF2.dataCH1);
	ynStage0.dataCH2 = alpha0 * (xn.dataCH2 - rho * z1_LPF1.dataCH2 - z1_LPF2.dataCH2);

	//std::cout << "odmkSEMSVF: data in = " << xn << ",    rho = " << rho
	//			<< ",    Z11 = " << z11 << ",    Z12 = " << z12 << ",    ynHP = " << ynStage0 << std::endl;

	// BPF Out
	ynStage1.dataCH1 = alpha * ynStage0.dataCH1 + z1_LPF1.dataCH1;
	ynStage1.dataCH2 = alpha * ynStage0.dataCH2 + z1_LPF1.dataCH2;

	//std::cout<<"odmkSEMSVF: Band-Pass ynBP = "<<ynStage1<<std::endl;

	// for nonlinear proc
//	if(NLP == ON)
//		ynBP = fasttanh(saturation * ynBP);

	// LPF Out
	ynStage2.dataCH1 = alpha * ynStage1.dataCH1 + z1_LPF2.dataCH1;
	ynStage2.dataCH2 = alpha * ynStage1.dataCH2 + z1_LPF2.dataCH2;

	ynHP.dataCH1 = ynStage0.dataCH1;
	ynHP.dataCH2 = ynStage0.dataCH2;

	ynBP.dataCH1 = ynStage1.dataCH1;
	ynBP.dataCH2 = ynStage1.dataCH2;

	ynLP.dataCH1 = ynStage2.dataCH1;
	ynLP.dataCH2 = ynStage2.dataCH2;

	// SEM Band-Stop Output
	// Fixed 1/2 & 1/2 mix for Band-Stop. Original uses AuxControl to modify mix
	ynBS.dataCH1 = (ynHP.dataCH1 + ynLP.dataCH1) / 2;
	ynBS.dataCH2 = (ynHP.dataCH2 + ynLP.dataCH2) / 2;

	// update memory
    z1_LPF1.dataCH1 = alpha * ynHP.dataCH1 + ynBP.dataCH1;
    z1_LPF1.dataCH2 = alpha * ynHP.dataCH2 + ynBP.dataCH2;

	z1_LPF2.dataCH1 = alpha * ynBP.dataCH1 + ynLP.dataCH1;
	z1_LPF2.dataCH2 = alpha * ynBP.dataCH2 + ynLP.dataCH2;

	// return our selected type
	if(fType == 3) {
		yn.dataCH1 = ynHP.dataCH1;
		yn.dataCH2 = ynHP.dataCH2;
	} else if(fType == 4) {
		yn.dataCH1 = ynBP.dataCH1;
		yn.dataCH2 = ynBP.dataCH2;
	} else if(fType == 5) {
		yn.dataCH1 = ynBS.dataCH1;
		yn.dataCH2 = ynBS.dataCH2;
	} else {
		yn.dataCH1 = ynLP.dataCH1;
		yn.dataCH2 = ynLP.dataCH2;
	}

	//std::cout << "xnL = " << xn << ",	fb1 = " << fb1 << ",	fb2 = " << fb2
	// 			<< ",	fb3 = " << fb3 << ",	fAlpha0 = " << fParam.alpha0
	//			<< ",	SM = "<<SM<<",	un = "<<un<<std::endl;

}

// *-------------------------------------------------------------------------------------------* //


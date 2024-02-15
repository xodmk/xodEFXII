/*------------------------------------------------------------------------------------------------*/
/* ___::((xodEFXII.cpp))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018))::___
   ___::((created by eschei))___

	Purpose: header .h for XODMK EFXII
	Device: All
	Revision History: September 1, 2017 - initial
	Revision History: May 9, 2018 - v4
	Revision History: 2022-01-21 -  v4 xodmk
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#include <stdint.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>

#include "xodEFXII_types.h"


///*------------------------------------------------------------------------------------------------*/
//
//template<int maxChannels, int maxDelayFDN>
//class DDLFDNFX {
//
//	ap_uint<1> ddlCross = 0;
//	ap_uint<1> extFB = 0;
//	ddlCtrlAll_t ddlCtrl;
//	ap_uint<1> houseMix = 0;
//	ap_uint<1> ssbFbMod = 0;
//
//	multiData_t feedbackIn {0};
//	multiData_t feedbackMux {0};
//	multiData_t feedbackOut {0};
//	multiData_t houseIn {0};
//	multiData_t houseOut {0};
//
//	multiData_t ddlxnOut {0};
//
//	stereoData_t ssbIn {0};
//	stereoData_t ssbOut {0};
//
//	xodDDLXN<maxChannels, maxDelayFDN> ddlXNfdn;
//	ssbHilbertMod ssbMod;
//
//
//public:
//
//	DDLFDNFX(ap_uint<1> selectCross, ap_uint<1> selectExtFB,
//			 ap_uint<1> selectHouse, ap_uint<1> selectSSB, ddlCtrlAll_t ddlCtrlAll) {
//
//		ddlCross = selectCross;
//		if (selectExtFB or selectHouse or selectSSB) {
//			extFB = 1;
//		} else {
//			extFB = 0;
//		}
//		houseMix = selectHouse;
//		ssbFbMod = selectSSB;
//		ddlCtrl.wetMix = ddlCtrlAll.wetMix;
//		ddlCtrl.feedbackMix = ddlCtrlAll.feedbackMix;
//		ddlCtrl.outputGain = ddlCtrlAll.outputGain;
//		for (int c = 0; c < maxChannels; ++c) {
//			ddlCtrl.delayLenMulti[c] = ddlCtrlAll.delayLenMulti[c];
//		}
//
//	}
//
//
//	void ddlFdnSet(ap_uint<1> selectCross, ap_uint<1> selectExtFB, ap_uint<1> selectHouse,
//			       ap_uint<1> selectSSB, ddlCtrlAll_t ddlCtrlAll) {
//		ddlCross = selectCross;
//		if (selectExtFB or selectHouse or selectSSB) {
//			extFB = 1;
//		} else {
//			extFB = 0;
//		}
//		houseMix = selectHouse;
//		ssbFbMod = selectSSB;
//		ddlCtrl.wetMix = ddlCtrlAll.wetMix;
//		ddlCtrl.feedbackMix = ddlCtrlAll.feedbackMix;
//		ddlCtrl.outputGain = ddlCtrlAll.outputGain;
//		for (int c = 0; c < maxChannels; ++c) {
//			ddlCtrl.delayLenMulti[c] = ddlCtrlAll.delayLenMulti[c];
//		}
//
//	}
//
//
//	void advance(ddsStep_t ssbModOscStep, multiData_t ddlFdnIn, multiData_t &ddlFdnOut) {
//
//		ddlXNfdn.advance(ddlCross, extFB, ddlCtrl, ddlFdnIn,
//				         feedbackIn, feedbackOut, ddlxnOut);
//
//		if (houseMix == 1) {
//			// Mix using a Householder matrix
//			//Householder<dataExt_t>::house(ddlOut, ddlMixed);
//			//Householder<dataExt_t>::house(FB, FBMixed);
//
////#ifndef __SYNTHESIS__
////			// TEMP PROBE it
////			static int houseCnt = 0;
////			houseCnt++;
////#endif
//
//			houseIn_loop: for (int c = 0; c < maxChannels; ++c) {
//#pragma HLS UNROLL
//				houseIn[c] = feedbackOut[c];
//
////#ifndef __SYNTHESIS__
////				// TEMP PROBE it
////				if (houseCnt >= PROBE_LOW && houseCnt <= PROBE_HIGH) {
////					std::cout << "xodReverbII.h[" << houseCnt << "]: houseIn[" << c << "] = " << static_cast<float>(houseIn[c]) << std::endl;
////				}
////#endif
//
//
//			}
//
//			// mix feedback delay network with householder function
//			Householder<dataExt_t, static_cast<int>(maxChannels)>::house(houseIn, houseOut);
//
//			houseMix_loop: for (int c = 0; c < maxChannels; ++c) {
//#pragma HLS UNROLL
//				feedbackMux[c] = houseOut[c];
//
////#ifndef __SYNTHESIS__
////				// TEMP PROBE it
////				if (houseCnt >= PROBE_LOW && houseCnt <= PROBE_HIGH) {
////					std::cout << "xodReverbII.h[" << houseCnt << "]: houseOut[" << c << "] = " << static_cast<float>(houseOut[c]) << std::endl;
////				}
////#endif
//
//			}
//
//		} else {
//			feedback_loop: for (int c = 0; c < maxChannels; ++c) {
//#pragma HLS UNROLL
//				feedbackMux[c] = feedbackOut[c];
//			}
//		}
//
//
//		// sign extend 32bit ssbMod freq control to DDS phase acc width
//		ddsPhase_t ssbModFreqCtrl = static_cast<ddsPhase_t>(ssbModOscStep);
//
//		// TEMP
////		ssbIn_loop: for (int c = 0; c < maxChannels; ++c) {
////#pragma HLS UNROLL
////			ssbIn[c] = feedbackMux[c];
////		}
//
//		// TEMP 2 Channel design only
//		ssbIn.dataCH1 = feedbackMux[0];
//		ssbIn.dataCH2 = feedbackMux[1];
//
//		ssbOut = ssbMod.advance(ssbIn, ssbModFreqCtrl);
//
//		// TEMP
//		ssbFb_loop: for (int c = 0; c < maxChannels; ++c) {
//#pragma HLS UNROLL
//			if (ssbFbMod == 1) {
//				//feedbackIn[c] = ssbOut[c];
//				feedbackIn[0] = ssbOut.dataCH1;
//				feedbackIn[1] = ssbOut.dataCH2;
//
//			} else {
//				feedbackIn[c] = feedbackMux[c];
//			}
//		}
//
//		output_loop: for (int c = 0; c < maxChannels; ++c) {
//#pragma HLS UNROLL
//			ddlFdnOut[c] = ddlxnOut[c];
//		}
//	}
//
//};


/*------------------------------------------------------------------------------------------------*/



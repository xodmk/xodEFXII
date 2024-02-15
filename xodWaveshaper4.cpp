/*------------------------------------------------------------------------------------------------*/
/* ___::((xodWaveshaper4.cpp))::___

   ___::((created by eschei))___

	Purpose: xodWaveshaper4
	Device: All
	Revision History: June 8, 2018 - initial
	Revision History: July 4, 2018 - version 2
	Revision History: July 5, 2018 - version 3
	Revision History: 2022-03-09   - XODMK version 4
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
 *---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#include <cassert>

//#include "hls_math.h"

#include "xodEFXII_types.h"
#include "xodEFXII.h"
#include "xodWaveshaper4.h"



/* ---------------------------------------------------------- */

static void xodATANlin(atanIn_t x, atanOut_t& y);
static void xodATANgain(atanIn_t x, wavshaperGain_t& g);

/* ---------------------------------------------------------- */

//template <class T, class F>
//T linInterpolate(T dataLow, T dataHigh, F fractional)
//{
//	T linInterpOut;
//	linInterpOut = dataLow + (dataHigh - dataLow) * fractional;
//	return linInterpOut;
//}



data_t saturator(data_t din) {
	data_t dout = 0;

	if (din > (data_t)1.0) {
		dout = (data_t)1.0;
	} else if (din < (data_t)(-1.0)) {
		dout = (data_t)(-1.0);
	} else {
		dout = din;
	}

	return dout;
}


static void xodATANlin(atanIn_t x, atanOut_t& y) {
#pragma HLS PIPELINE

	atanOut_t yLow, yHigh = 0;
	atanIn_t xabs = 0;

	ap_uint<3> muxSel = 0;
	ap_uint<5> tblAddr = 0;

	// atanIn_t = ap_fixed<27,11> - worst case datain = 1023
	assert(x < 1024 && x > -1023);

	if (x.is_neg()) {
		xabs = -x;
	} else {
		xabs = x;
	}
	ap_ufixed<27, 19> xScale = xabs * 2;	// left shift by 1 so that LSB maps (x:0-8 -> addr:0-15)
	ap_uint<19> xUInt = static_cast<ap_uint<19> >(xScale);
	ap_ufixed<15, 0> fracX = static_cast<ap_ufixed<15, 0> >(xabs);	// << 12

	//printf("\nxodATANlin: x = %f,    fracX = %f,    xScale = %f,    xUInt = %i\n", (float)x, (float)fracX, (float)xScale, (int)xUInt);

	if (xUInt(18,10) >= 1) {								// x >= 512
		muxSel = 5;
	} else if (xUInt(10,4) == 0) {							// x < 8 => partition 1
		muxSel = 0;
	} else if (xUInt(10,4) == 1) {							// 8 <= x < 16 => partition 2
		muxSel = 1;
	} else if (xUInt(10,5) == 1) {							// 16 <= x < 32 => partition 3
		muxSel = 2;
	} else if (xUInt(10,6) >= 1 && xUInt(10,6) < 4) {		// 32 <= x < 128 => partition 4
		muxSel = 3;
	} else if (xUInt(10,8) > 0) {							// 128 <= x < 512 => partition 5
		muxSel = 4;
	} else {
		muxSel = 0;
	}

	//printf("\nodmkATANpoly: x(26,11) = %i,    muxSel = %i,    x(13,8) = %i\n", (int)xUInt(26,11), (int)muxSel, (int)xUInt(13,8));

	switch(muxSel) {
		case 0: {
			tblAddr = static_cast<ap_uint<5> >(xUInt(3,0));	break;			// maps x[0:7] -> addr[0:15]
		}
		case 1: {
			tblAddr = static_cast<ap_uint<5> >(xUInt(4,1) + 8); break;		// maps x[8:15] -> addr[16:23]
		}
		case 2: {
			tblAddr = static_cast<ap_uint<5> >(xUInt(5,3) + 20); break;		// maps x[16:32] -> addr[24:27]
		}
		case 3: {
			tblAddr = static_cast<ap_uint<5> >(28); break;
		}
		case 4: {
			tblAddr = static_cast<ap_uint<5> >(29); break;
		}
		case 5: {
			tblAddr = static_cast<ap_uint<5> >(30); break;
		}
		default:
			break;
	}

	//printf("odmkATANpoly tbAddr = %i\n", (int)tbAddr);
	//printf("odmkATANpoly a0[tbAddr] = %f,    a1[tbAddr] = %f\n", (float)a0[tbAddr], (float)a1[tbAddr]);


	if (x < 0) {
		yLow = static_cast<atanOut_t>(-((a1[tblAddr] * xabs + a0[tblAddr])));
		yHigh = static_cast<atanOut_t>(-((a1[tblAddr+1] * xabs + a0[tblAddr+1])));
	} else {
		yLow = static_cast<atanOut_t>(a1[tblAddr] * xabs + a0[tblAddr]);
		yHigh = static_cast<atanOut_t>(a1[tblAddr+1] * xabs + a0[tblAddr+1]);
	}

	y = linInterpolate(yLow, yHigh, fracX);	// linear interpolation function

	//printf("xodATANlin yLow[%i] = %f,    yHigh[%i] = %f,    y = %f\n", (int)tblAddr, (float)yLow, (int)tblAddr+1, (float)yHigh, (float)y);

}

//ap_ufixed<16,10> / ap_fixed<27,12>
static void xodATANgain(atanIn_t x, wavshaperGain_t& g) {
#pragma HLS PIPELINE

	atanIn_t xabs = 0;

	wavshaperGain_t gainLow, gainHigh = 0;

	ap_uint<3> muxSel = 0;
	ap_uint<5> tblAddr = 0;

	// atanIn_t = ap_fixed<27,11> - worst case datain = 1023
	assert(x < 1024 && x > -1023);

	ap_fixed<27, 20> xScale = x * 2;	// left shift by 1 so that LSB maps (x:0-8 -> addr:0-15)
	ap_uint<19> xUInt = static_cast<ap_uint<19> >(static_cast<int>(xScale));

	ap_ufixed<15, 0> fracX = static_cast<ap_ufixed<15, 0> >(x);	// << 12

	//printf("\nxodATANlin: x = %f,    fracX = %f,    xScale = %f,    xUInt = %i\n", (float)x, (float)fracX, (float)xScale, (int)xUInt);

	if (xUInt(18,10) >= 1) {								// x >= 512
		muxSel = 5;
	} else if (xUInt(10,4) == 0) {							// x < 8 => partition 1
		muxSel = 0;
	} else if (xUInt(10,4) == 1) {							// 8 <= x < 16 => partition 2
		muxSel = 1;
	} else if (xUInt(10,5) == 1) {							// 16 <= x < 32 => partition 3
		muxSel = 2;
	} else if (xUInt(10,6) >= 1 && xUInt(10,6) < 4) {		// 32 <= x < 128 => partition 4
		muxSel = 3;
	} else if (xUInt(10,8) > 0) {							// 128 <= x < 512 => partition 5
		muxSel = 4;
	} else {
		muxSel = 0;
	}

	//printf("\nodmkATANpoly: x(26,11) = %i,    muxSel = %i,    x(13,8) = %i\n", (int)xUInt(26,11), (int)muxSel, (int)xUInt(13,8));

	switch(muxSel) {
		case 0: {
			tblAddr = static_cast<ap_uint<5> >(xUInt(3,0));	break;			// maps x[0:7] -> addr[0:15]
		}
		case 1: {
			tblAddr = static_cast<ap_uint<5> >(xUInt(4,1) + 8); break;		// maps x[8:15] -> addr[16:23]
		}
		case 2: {
			tblAddr = static_cast<ap_uint<5> >(xUInt(5,3) + 20); break;		// maps x[16:32] -> addr[24:27]
		}
		case 3: {
			tblAddr = static_cast<ap_uint<5> >(28); break;
		}
		case 4: {
			tblAddr = static_cast<ap_uint<5> >(29); break;
		}
		case 5: {
			tblAddr = static_cast<ap_uint<5> >(30); break;
		}
		default:
			break;
	}

	//printf("odmkATANpoly tbAddr = %i\n", (int)tbAddr);
	//printf("odmkATANpoly a0[tbAddr] = %f,    a1[tbAddr] = %f\n", (float)a0[tbAddr], (float)a1[tbAddr]);

	gainLow = static_cast<wavshaperGain_t>(wsGainTbl[tblAddr]);
	gainHigh = static_cast<wavshaperGain_t>(wsGainTbl[tblAddr+1]);

	g = linInterpolate(gainLow, gainHigh, fracX);

}


// *--------------------------------------------------------* //
// *--- xodWaveshaper4 HLS functions ---* //


void xodWaveshaper4::advance(ap_ufixed<16,10> wsGain, stereoData_t waveshaperIn,
		                     stereoData_t& waveshaperOut) {

//#pragma HLS DATAFLOW

	stereoData_t waveshaperTmp;

	atanOut_t wsAtanCH1, wsAtanCH2 = 0;
	wavshaperGain_t wsGainScl = 0;
	atanIn_t wsGainTmp = static_cast<atanIn_t>(wsGain);

	xodATANgain(wsGainTmp, wsGainScl);

	atanIn_t wsIn1_x_G = static_cast<atanIn_t>(wsGain * waveshaperIn.dataCH1);
	atanIn_t wsIn2_x_G = static_cast<atanIn_t>(wsGain * waveshaperIn.dataCH2);
	xodATANlin(wsIn1_x_G, wsAtanCH1);
	xodATANlin(wsIn2_x_G, wsAtanCH2);

	waveshaperTmp.dataCH1 = static_cast<data_t>(wsAtanCH1 * wsGainScl);
	waveshaperTmp.dataCH2 = static_cast<data_t>(wsAtanCH2 * wsGainScl);

	waveshaperOut.dataCH1 = saturator(waveshaperTmp.dataCH1);
	waveshaperOut.dataCH2 = saturator(waveshaperTmp.dataCH2);

	// TEMP DEBUG
//	atanOut_t tempWsGainAtan;
//	gain_t tempWsGainGain;
//	xodATANlin(static_cast<atanIn_t>(wsGain), tempWsGainAtan);
//	std::cout << "wsIn = " << static_cast<float>(waveshaperIn.dataCH1)
//			  << ",    wsGain = " << static_cast<float>(static_cast<atanIn_t>(wsGain))
//			  << ",    wsGainScl = " << static_cast<float>(wsGainScl)
//			  << ",    wsIn1_x_G = " << static_cast<float>(wsIn1_x_G)
//			  << ",    xodATAN(wsGain) = " << static_cast<float>(tempWsGainAtan)
//			  << ",    xodATAN(wsIn1_x_G) = " << static_cast<float>(wsAtanCH1)
//			  << ",    wsOut = " << static_cast<float>(waveshaperOut.dataCH1) << std::endl;

	return;
}

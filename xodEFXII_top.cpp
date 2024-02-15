/*------------------------------------------------------------------------------------------------*/
/* ___::((xodEFXII_top.cpp))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018:2023))::___
   ___::((created by eschei))___

	Purpose: top-level cpp for XODMK EFXII
	Device: All
	Revision History: September 1, 2017 - initial
	Revision History: May 9, 2018 - v4
	Revision History: 2022-02-9 -  v4 xodmk
	Revision History: 2022-02-14 - DDL5 update
	Revision History: 2022-09-18 - update with DDLXN & multi-channels
	Revision History: 2022-09-21 - update with XodSSBMod
	Revision History: 2023-07-07 - Added LFO Modulation functions
	Revision History: 2023-09-09 - updated FPGA HW Bus Interface CTRL
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*

  XODMK EFX - FPGA Audio Effects Processing

  ::UI::

   	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.93;
	ddlCTRL.feedbackMix = 0.9;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 3560.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 93;
    ssbModFreqCTRL.ssbTilt = 0;

    wavShaperGain = 56.6;

    efxOutGain = 0.93;

    lfoTypeCTRL.lfoTargCtrl[0] = SSBMODFREQ;
    lfoTypeCTRL.lfoTypeCtrl[0] = 0;
    lfoTypeCTRL.lfoTargCtrl[1] = SSBMODFREQ;
    lfoTypeCTRL.lfoTypeCtrl[1] = 0;

    lfoWavCTRL.lfoGainCtrl[0] = lfoGainScaleUI * 0.01;
    lfoWavCTRL.lfoFreqCtrl[0] = 5566;
    lfoWavCTRL.lfoGainCtrl[1] = lfoGainScaleUI * 0.01;
    lfoWavCTRL.lfoFreqCtrl[1] = 5566;

*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#include "xodDDLXN.h"
#include "xodLFOXN.h"
#include "xodWaveshaper4.h"

#include "xodEFXII_types.h"
#include "xodEFXII.h"


// *** Top-Level Ports / FPGA HW Interface ***
// axiCtrl 			=> 	axi4lite 32bit bus
// ddlGainBusCtrl 	=> 	<ddlFBGain[31:16], ddlWetDryMix[15:0]> (GPIO x 1)
// ddlLengthCtrl 	=> 	<ddlLengthCH3[95:64], ddlLengthCH3[63:32], ddlLengthCH3[31:0]> (3x32 GPIOx2)
// ssbModFreqCtrl 	=> 	<ddsFreqCtrlCH4[127:96], ddsFreqCtrlCH4[95:64], ddsFreqCtrlCH4[63:32], ddsFreqCtrlCH4[31:0]> (4x32 GPIOx2)
// wavShaperGain 	=> 	wavShaperGain[15:0]
// lfoWavBusCtrl 	=> 	<lfoFreqCtrl[15:0] x 2, lfoGainCtrl[15:0] x 2> (GPIO x 2)
// efxOutGainCtrl 	=> 	efxOutGainCtrl[15:0] (ddlOutputGain)

extern void xodEFXII_top(uint32_t* axiCtrl,
			 	 	 	 ddlGainBus_t ddlGainBusCtrl,
						 ddlLength4CHAXI_t ddlLengthCtrl,
						 ddsFreq4CHAXI_t ssbModFreqCtrl,
						 wavshaperCtrl_t wavShaperGain,
						 lfoWavBus_t lfoWavBusCtrl,
						 uint16_t efxOutGainCtrl,
						 axis_t &efxInAxis,
						 axis_t &efxOutAxis)
{
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE s_axilite port=axiCtrl bundle=AXI_CTRL_I
#pragma HLS INTERFACE axis port=efxInAxis
#pragma HLS INTERFACE axis port=efxOutAxis

#pragma HLS aggregate variable=ddlGainBusCtrl compact=bit
#pragma HLS aggregate variable=ddlLengthCtrl compact=bit
#pragma HLS aggregate variable=ssbModFreqCtrl compact=bit
#pragma HLS aggregate variable=lfoWavBusCtrl compact=bit


	// *--------------------------------------------------------------------------------* //

	stereoData_t efxInStereo {0};
	multiData_t fdnFxIn {0};
	multiData_t fdnFxOut {0};

	multiData_t ddlMixed {0};

	stereoDataExt_t fdnFxStereoMix {0};
	stereoDataExt_t fdnFxStereoExt {0};
	stereoData_t fdnFxStereoOut {0};

	static stereoData_t ssbIn {0};
	static stereoData_t ssbOut {0};
	static stereoData_t feedbackIn {0};
	static stereoData_t feedbackOut {0};

	stereoData_t waveshaperOut {0};

	stereoData_t efxStereoOutPre {0};
	stereoData_t efxStereoOutFltr {0};
	stereoData_t efxStereoOutFX {0};
	stereoData_t efxStereoOut {0};

	multiLfoTarget_t lfoTargetCtrl {0};
	multiLfoShape_t lfoType {0};
	multiLfoData_t lfoOut;
	ap_uint<1> socOut[LFOCHANNELS];

	delayLen_t ddlLfoScaleVal = 0;
	ddsCtrl_t ssbLfoScaleVal = 0;

	ddlLenArray_t ddlLengthArray {0};
	ddsCtrl_t ssbModFreqArray[MAXCHANNELS] {0};

	switch_t switches {0};
	switchLFO_t switches {0};

	// *--------------------------------------------------------------------------------* //
	// *--- Read Switches ---*

	// Switch Values read from axiCtrl[23:16]
	switches.bypass      = static_cast<ap_uint<1> >(0x0001 & (*axiCtrl >> 16));
	switches.filterOn    = static_cast<ap_uint<1> >(0x0001 & (*axiCtrl >> 17));
	switches.wavShaperOn = static_cast<ap_uint<1> >(0x0001 & (*axiCtrl >> 18));
	switches.multiChan   = static_cast<ap_uint<1> >(0x0001 & (*axiCtrl >> 19));
	switches.selectCross = static_cast<ap_uint<1> >(0x0001 & (*axiCtrl >> 20));
	switches.selectHouse = static_cast<ap_uint<1> >(0x0001 & (*axiCtrl >> 21));
	switches.selectSSB   = static_cast<ap_uint<1> >(0x0001 & (*axiCtrl >> 22));
	//switches.reserve = static_cast<ap_uint<1> >(0x0001 & (*axiCtrl >> 23));
	// Switch Values read from axiCtrl[25:24]
	switchesLFO.lfoSync  = static_cast<ap_uint<1> >(0x0001 & (*axiCtrl >> 24));
	switchesLFO.lfoUpDn  = static_cast<ap_uint<1> >(0x0001 & (*axiCtrl >> 25));

	// *--------------------------------------------------------------------------------* //
	// Temp: Fixed 4-Channel Implementation because aggregated AXI-Lite bus 4CH types..
	// replace 4CH structs with N-CHANNEL concatenated words??

	ap_ufixed<24, 24> ddlLength4CH[MAXCHANNELS];
	// CH1 24lsb
	ddlLength4CH[0] = static_cast<ap_ufixed<24, 24> >(ddlLengthCtrl.ddlLengthCH1);
	// CH2 16lsb << 8 & CH1 8msb ( 16776960 = 111111111111111100000000 )
	ddlLength4CH[1] = static_cast<ap_ufixed<24, 24> >( (static_cast<ap_ufixed<24, 24> >(ddlLengthCtrl.ddlLengthCH2 << 8)
												      & static_cast<ap_ufixed<24, 24> >(16776960))
												      | static_cast<ap_ufixed<24, 24> >(ddlLengthCtrl.ddlLengthCH1 >> 24));

	// CH3 8lsb << 16 & CH2 16msb ( 16711680 = 111111110000000000000000, 65535 = 1111111111111111 )
	ddlLength4CH[2] = static_cast<ap_ufixed<24, 24> >( ((static_cast<ap_ufixed<24, 24> >(ddlLengthCtrl.ddlLengthCH3 << 16)
												      & static_cast<ap_ufixed<24, 24> >(16711680)))
												      | (static_cast<ap_ufixed<24, 24> >(ddlLengthCtrl.ddlLengthCH2 >> 16)
												      & static_cast<ap_ufixed<24, 24> >(65535)) );

	ddlLength4CH[3] = static_cast<ap_ufixed<24, 24> >(ddlLengthCtrl.ddlLengthCH3 >> 8);

	ap_ufixed<24, 16> ddlLengthTmp[MAXCHANNELS];
	ddlLengthTmp[0] = static_cast<ap_ufixed<24, 16> >(static_cast<ap_ufixed<32, 24> >(ddlLength4CH[0]) >> 8);
	ddlLengthTmp[1] = static_cast<ap_ufixed<24, 16> >(static_cast<ap_ufixed<32, 24> >(ddlLength4CH[1]) >> 8);
	ddlLengthTmp[2] = static_cast<ap_ufixed<24, 16> >(static_cast<ap_ufixed<32, 24> >(ddlLength4CH[2]) >> 8);
	ddlLengthTmp[3] = static_cast<ap_ufixed<24, 16> >(static_cast<ap_ufixed<32, 24> >(ddlLength4CH[3]) >> 8);

//#ifndef __SYNTHESIS__
//	//TEMP PROBE it
//	static int efxDataCnt = 0;
//	efxDataCnt++;
//
//	//TEMP PROBE it
//	if (efxDataCnt >= PROBE_LOW && efxDataCnt <= PROBE_HIGH) {
////		for (int i = 0; i < static_cast<int>(MAXCHANNELS); i++) {
////			std::cout << "xodEFX_top - ddlLengthArray[" << efxDataCnt << "]: " << ddlLengthArray[i] << std::endl;
////		}
//		std::cout << "xodEFX_top[" << efxDataCnt << "] - ddlLengthCH1: " << ddlLengthCtrl->ddlLengthCH1 << std::endl;
//		std::cout << "xodEFX_top[" << efxDataCnt << "] - ddlLengthCH2: " << ddlLengthCtrl->ddlLengthCH2 << std::endl;
//		std::cout << "xodEFX_top[" << efxDataCnt << "] - ddlLengthCH3: " << ddlLengthCtrl->ddlLengthCH3 << std::endl;
//
//		std::cout << "xodEFX_top[" << efxDataCnt << "] - ddlLengthArray[0]: " << ddlLengthArray[0] << std::endl;
//		std::cout << "xodEFX_top[" << efxDataCnt << "] - ddlLengthArray[1]: " << ddlLengthArray[1] << std::endl;
//		std::cout << "xodEFX_top[" << efxDataCnt << "] - ddlLengthArray[2]: " << ddlLengthArray[2] << std::endl;
//		std::cout << "xodEFX_top[" << efxDataCnt << "] - ddlLengthArray[3]: " << ddlLengthArray[3] << std::endl;
//	}
//	else if (efxDataCnt == 713841) {
//		efxDataCnt = 0;
//	}
//#endif

	ddsCtrl_t ssbModFreqTmp[MAXCHANNELS];
	ssbModFreqTmp[0] = static_cast<ddsCtrl_t>(ssbModFreqCtrl.ddsFreqCtrlCH1);
	ssbModFreqTmp[1] = static_cast<ddsCtrl_t>(ssbModFreqCtrl.ddsFreqCtrlCH2);
	ssbModFreqTmp[2] = static_cast<ddsCtrl_t>(ssbModFreqCtrl.ddsFreqCtrlCH3);
	ssbModFreqTmp[3] = static_cast<ddsCtrl_t>(ssbModFreqCtrl.ddsFreqCtrlCH4);

	// *--------------------------------------------------------------------------------* //

	// convert from ddlGainBus_t to delayCtrl_t (FPGA HW Bus data -> internal Fixed point Ctrl variables)
	ddlCtrlAll_t ddlCtrlAll;
	delayCtrl_t ddlWetMix 		= uintxToUFixed_1p0<uint16_t, delayCtrl_t, 16>(ddlGainBusCtrl.uWetMix);
	delayCtrl_t ddlFeedbackMix 	= uintxToUFixed_1p0<uint16_t, delayCtrl_t, 16>(ddlGainBusCtrl.uFeedbackMix);
	ddlCtrlAll.wetMix		= ddlWetMix;
	ddlCtrlAll.feedbackMix 		= ddlFeedbackMix;

	static DDLFDNFX<static_cast<int>(MAXCHANNELS), static_cast<int>(MAXDELAY)> FDNFX(switches.selectCross, switches.selectHouse,
			        							 switches.selectSSB, ddlLengthArray, ddlCtrlAll);

	//static xodDDLXN DDL;
	static xodWaveshaper4 wavShape1;

	delayCtrl_t efxOutputGain 	= uintxToUFixed_1p0<uint16_t, delayCtrl_t, 16>(efxOutGainCtrl);

	// *--------------------------------------------------------------------------------* //
    // Prepare Input Channel Data

	// Duplicate input channels as many times as needed
	efxInStereo = efxInAxis.read();
	assert(efxInStereo.dataCH1 <=  static_cast<data_t>(1.0) && efxInStereo.dataCH2 <=  static_cast<data_t>(1.0));
	assert(efxInStereo.dataCH1 >= static_cast<data_t>(-1.0) && efxInStereo.dataCH2 >= static_cast<data_t>(-1.0));

//#ifndef __SYNTHESIS__
//	//TEMP PROBE it - Check LFO signals
//	static int efxDataCnt = 0;
//	if (switches.lfoSync == 1) {
//		efxDataCnt = 0;
//	}
//	efxDataCnt++;
//#endif

	duplicator_loop: for (int c = 0; c < MAXCHANNELS; ++c) {
#pragma HLS UNROLL
		int inputChannel = c % 2;
		if (inputChannel == 0) {
			fdnFxIn[c] = static_cast<dataExt_t>(efxInStereo.dataCH2);
		} else {
			fdnFxIn[c] = static_cast<dataExt_t>(efxInStereo.dataCH1);
		}

//#ifndef __SYNTHESIS__
//		//TEMP PROBE it
//		if (efxDataCnt >= PROBE_LOW && efxDataCnt <= PROBE_HIGH) {
//			if (c == 1) {
//				std::cout << "xodEFX_top[" << efxDataCnt << "]: ";
//			}
//			if (c == 1 || c == 2) {
//				std::cout << "   fdnFxIn[" << c << "] = " << static_cast<float>(fdnFxIn[c]);
//			}
//			if (c == 2) {
//				std::cout << std::endl;
//			}
//		}
//#endif

	}

	// *--------------------------------------------------------------------------------* //
    // Advance Modulation

	// Assumes 2-Channel LFOs for current hardware implementation
	// Read lfoShape & lfoTarget settings from AXI4lite bus
	// AXI4lite bus: uint32_t* efxCtrl_I;
	// AXI4lite bus: { ap_uint<3> shapeCH1, ap_uint<3> shapeCH2, ap_uint<4> targetCH1, ap_uint<4> targetCH2 }
	lfoInit_loop: for (int c = 0; c < LFOCHANNELS; ++c) {
#pragma HLS UNROLL
		lfoType[c] = static_cast<ap_uint<3> >(0x0007 & (*axiCtrl >> (c * 3)));	// *** CHECKIT ***
		lfoTargetCtrl[c] = static_cast<ap_uint<4> >(0x000F & (*axiCtrl >> (c * 4 + 6)));
	}

	// convert from lfoWavBus_t to lfoWavCtrl_t (FPGA HW Bus data -> internal Fixed point Ctrl variables)
	lfoWavCtrl_t lfoWavCtrl;
	lfoWavCtrl.lfoGainCtrl[0] = static_cast<lfoGain_t>(lfoWavBusCtrl.uLfoGainCtrlCH1);
	lfoWavCtrl.lfoFreqCtrl[0] = static_cast<lfoGain_t>(lfoWavBusCtrl.uLfoFreqCtrlCH1);
	lfoWavCtrl.lfoGainCtrl[1] = static_cast<lfoGain_t>(lfoWavBusCtrl.uLfoGainCtrlCH2);
	lfoWavCtrl.lfoFreqCtrl[1] = static_cast<lfoGain_t>(lfoWavBusCtrl.uLfoFreqCtrlCH2);

	xodLfoXN(switches.lfoSync, switches.lfoUpDn, lfoType, lfoWavCtrl.lfoFreqCtrl, lfoOut, socOut);

	// reverse loop => CH0 priority for duplicate targets
	lfoCtrl_loop: for (int m = LFOCHANNELS-1; m >= 0; --m) {
#pragma HLS UNROLL

		if (lfoTargetCtrl[m] == 2) {		// 0010 = DDLLENGTHBASE

			// __** DDLLENGTHBASE LFO Scaling: **__
			//
			// Final MAX DDLLENGTH Range: [0:2**DLYBWIDTH - 1]  (for DLYBWIDTH 16bits => [0:65535] )
			// lfoOut Range: [-1.0:1.0] - (lfoData_t = ap_fixed<lfoDataWidth, 2> => +/-1.0)
			// lfoGainCtrl Range: [0:2**lfoGainWidth-1] => [0:65535] (lfoGainWidth = 16bits)
			// worst case:  +/- 65535 when current DDLLENGTH = 0, otherwise, limit LFO Gain with UI CTRL
			// LfoScaleVal = current-lfo-val * (lfogain[0:1] * (2**lfoGainWidth-1)
			// LfoScaleVal = current-lfo-val * (lfogain[0:1] * 65535)
			ddlLfoScaleVal = static_cast<delayLen_t>(lfoOut[m] * lfoWavCtrl.lfoGainCtrl[m]);

		} else if (lfoTargetCtrl[m] == 3) {		// 0011 - SSBFREQBASE

			// __** SSBFREQBASE LFO Scaling: **__
			//
			// FINAL MAX ssbModFreq Range: [ -2**(ddsControlWidth-1)-1 : 2**(ddsControlWidth-1)-1]
			// FINAL MAX ssbModFreq Range: [-2147483647:2147483647]  (for Width = 32bits => 2**31 - 1 = 2147483647)
			// SSBOSC UI CTRL min/max freq = +/-1499 Hz  (for internal phase ACC = 36 bits, sr = 48000)
			// 1499 * (2**36 / 48000) = 2146051992 (OK!)  =>  1500 * (2**36 / 48000) = 2147483648.0 (***OVERFLOW***)
			//
			// warning: when using ssbTilt - tilt amount modifies worst case:
			// assert(ssbModFreqCTRL.ssbFreqBase + (abs(ssbModFreqCTRL.ssbTilt)) <= 1499);
			// assert(ssbModFreqCTRL.ssbFreqBase - (abs(ssbModFreqCTRL.ssbTilt)) >= -1499);
			//
			// lfoOut Range: ** Same as above case **
			// lfoGainCtrl Range: [0:65535]
			//
			// LfoScaleVal = current-lfo-val * (lfogain[0:1] * (2**lfoGainWidth-1)) * (2**(ddsControlWidth - lfoGainWidth - 1))
			// LfoScaleVal = current-lfo-val * (lfogain[0:1] * (2**16 - 1)) * (2**(31-16))
			// LfoScaleVal = current-lfo-val * lfoGainCtrl * 32768
			ssbLfoScaleVal = static_cast<ddsCtrl_t>(lfoOut[m] * lfoWavCtrl.lfoGainCtrl[m] * 32768);

		} else {
			ddlLfoScaleVal = 0;
			ssbLfoScaleVal = 0;
		}
	}

	// modulate, scale and saturate ddlLength /  modulate ssbModFreq arrays
	modGain_loop: for (int c = 0; c < MAXCHANNELS; ++c) {
#pragma HLS UNROLL
		ap_ufixed<32, 16> ddlLengthModTmp = static_cast<ap_ufixed<32, 16> >(ddlLengthTmp[c] + ddlLfoScaleVal);
		delayLen_t ddlLengthMod = (ddlLengthModTmp > MAXDELAY) ? static_cast<delayLen_t>(MAXDELAY) :
																 static_cast<delayLen_t>(ddlLengthModTmp);
		ddlLengthArray[c] = static_cast<delayLen_t>(ddlLengthMod);

		// Check FINAL ssbModFreq Range: [-2147483647:2147483647]  (for ddsControlWidth = 32)
		assert(static_cast<float>(ssbModFreqTmp[c] + ssbLfoScaleVal) <=  2147483647);
		assert(static_cast<float>(ssbModFreqTmp[c] + ssbLfoScaleVal) >= -2147483647);
		ssbModFreqArray[c] = static_cast<ddsCtrl_t>(ssbModFreqTmp[c] + ssbLfoScaleVal);
	}

//#ifndef __SYNTHESIS__
//	// TEMP PROBE it - Check LFO signals
//	if (efxDataCnt >= PROBE_LOW && efxDataCnt <= PROBE_HIGH) {
//		std::cout << "xodEFX_top[" << efxDataCnt << "]:   lfoOut[1] = " << static_cast<float>(lfoOut[1])
//				  << ",   ssbLfoScaleVal = " << static_cast<float>(ssbLfoScaleVal)
//				  << ",   ssbModFreqCtrl->ddsFreqCtrlCH4 = " << static_cast<float>(ssbModFreqCtrl.ddsFreqCtrlCH4)
//				  << ",   ssbModFreqArray[3] = " << static_cast<float>(ssbModFreqArray[3])
//				  << std::endl;
//	}
//#endif

	// *--------------------------------------------------------------------------------* //
    // advance EFX

	FDNFX.ddlFdnSet(switches.selectCross, switches.selectHouse, switches.selectSSB, ddlLengthArray, ddlCtrlAll);
	FDNFX.advance(ssbModFreqArray, fdnFxIn, fdnFxOut);

	// *--------------------------------------------------------------------------------* //
	// EFX Output mix

	if (switches.multiChan) {

		fdnFxStereoMix.dataCH1 = 0;
		fdnFxStereoMix.dataCH2 = 0;
		fdnFxStereoExt.dataCH1 = 0;
		fdnFxStereoExt.dataCH2 = 0;

		// Mix down into stereo for example output
		stereoMix_loop: for (int c = 0; c < MAXCHANNELS; c += 2) {
#pragma HLS UNROLL
			fdnFxStereoMix.dataCH1 += fdnFxOut[c];
			fdnFxStereoMix.dataCH2 += fdnFxOut[c + 1];
		}

		// constexpr double scaling = 2.0 / MAXCHANNELS;
		fdnFxStereoExt.dataCH1 = static_cast<dataExt_t>(fdnFxStereoMix.dataCH1 * channelScaling);
		fdnFxStereoExt.dataCH2 = static_cast<dataExt_t>(fdnFxStereoMix.dataCH2 * channelScaling);

	} else {
		fdnFxStereoExt.dataCH1 = fdnFxOut[static_cast<int>(MAXCHANNELS / 2)];
		fdnFxStereoExt.dataCH2 = fdnFxOut[static_cast<int>(MAXCHANNELS / 2 - 1)];
	}

	// CHECK output max values
	dataExt_t doutCheck = static_cast<dataExt_t>(2.0);
	assert(fdnFxStereoExt.dataCH1 <=  doutCheck && fdnFxStereoExt.dataCH2 <=  doutCheck);
	assert(fdnFxStereoExt.dataCH1 >= -doutCheck && fdnFxStereoExt.dataCH2 >= -doutCheck);

	fdnFxStereoOut.dataCH1 = (fdnFxStereoExt.dataCH1 > static_cast<dataExt_t>( 1.0)) ? static_cast<dataExt_t>( 1.0) : fdnFxStereoExt.dataCH1;
	fdnFxStereoOut.dataCH1 = (fdnFxStereoExt.dataCH1 < static_cast<dataExt_t>(-1.0)) ? static_cast<dataExt_t>(-1.0) : fdnFxStereoExt.dataCH1;

	fdnFxStereoOut.dataCH2 = (fdnFxStereoExt.dataCH2 > static_cast<dataExt_t>( 1.0)) ? static_cast<dataExt_t>( 1.0) : fdnFxStereoExt.dataCH2;
	fdnFxStereoOut.dataCH2 = (fdnFxStereoExt.dataCH2 < static_cast<dataExt_t>(-1.0)) ? static_cast<dataExt_t>(-1.0) : fdnFxStereoExt.dataCH2;

	// *--------------------------------------------------------------------------------* //
	// Waveshaper Processing

	wavShape1.advance(wavShaperGain, fdnFxStereoOut, waveshaperOut);

	assert(waveshaperOut.dataCH1 <= static_cast<data_t>( 1.0) && waveshaperOut.dataCH2 <= static_cast<data_t>( 1.0));
	assert(waveshaperOut.dataCH1 >= static_cast<data_t>(-1.0) && waveshaperOut.dataCH2 >= static_cast<data_t>(-1.0));

	// *--------------------------------------------------------------------------------* //
	// Output Mux

	if (switches.bypass == 1) {
		efxStereoOutPre.dataCH1 = efxInStereo.dataCH1;
		efxStereoOutPre.dataCH2 = efxInStereo.dataCH2;
	} else {
		if (switches.wavShaperOn == 1) {
			efxStereoOutPre.dataCH1 = waveshaperOut.dataCH1;
			efxStereoOutPre.dataCH2 = waveshaperOut.dataCH2;
		} else {
			efxStereoOutPre.dataCH1 = fdnFxStereoOut.dataCH1;
			efxStereoOutPre.dataCH2 = fdnFxStereoOut.dataCH2;
		}
		if (switches.filterOn == 1) {
			efxStereoOutFX.dataCH1 = efxStereoOutFltr.dataCH1;
			efxStereoOutFX.dataCH2 = efxStereoOutFltr.dataCH2;
		} else {
			efxStereoOutFX.dataCH1 = efxStereoOutPre.dataCH1;
			efxStereoOutFX.dataCH2 = efxStereoOutPre.dataCH2;
		}
	}

	efxStereoOut.dataCH1 = efxStereoOutFX.dataCH1 * efxOutputGain;
	efxStereoOut.dataCH2 = efxStereoOutFX.dataCH2 * efxOutputGain;

//#ifndef __SYNTHESIS__
//	//TEMP PROBE it
//	if (efxDataCnt >= PROBE_LOW && efxDataCnt <= PROBE_HIGH) {
//		std::cout << "xodEFX_top[" << efxDataCnt << "]:   efxStereoOut.dataCH1 = " << static_cast<float>(efxStereoOut.dataCH1)
//				  << ",   efxStereoOut.dataCH1 = " << static_cast<float>(efxStereoOut.dataCH1) << std::endl;
//	}
//#endif

	//assert(efxStereoOut.dataCH1 <=  1.0 && efxStereoOut.dataCH2 <=  1.0);
	//assert(efxStereoOut.dataCH1 >= -1.0 && efxStereoOut.dataCH2 >= -1.0);
	//assert(fdnFxStereoOut.dataCH1 <=  static_cast<data_t>(1.0) && fdnFxStereoOut.dataCH2 <=  static_cast<data_t>(1.0));
	//assert(fdnFxStereoOut.dataCH1 >= static_cast<data_t>(-1.0) && fdnFxStereoOut.dataCH2 >= static_cast<data_t>(-1.0));

	efxOutAxis.write(efxStereoOut);

	// *--------------------------------------------------------------------------------* //

	return;

}

/*------------------------------------------------------------------------------------------------*/
/* ___::((xodSsbHilbertMod.cpp))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018:2022))::___
   ___::((created by eschei))___

	Purpose: xodSsbHilbertMod - Single Side-band Modulation with Hilbert Transformer
	Device: All
	Revision History: July 14, 2017 - initial
	Revision History: May 12, 2018 - version 2
	Revision History: 2022-02-5 - xodmk
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
 *---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#include <ap_int.h>
#include "ap_shift_reg.h"

#include "xodSsbHilbertMod.h"
#include "xodEFX.h"


outStereoFIR_t stereoFir::advance(inStereoFIR_t firIn) {
//#pragma HLS PIPELINE

	outStereoFIR_t firOut;

	static ap_shift_reg<inDataFIR_t, numCoef> shiftReg_L;
	static ap_shift_reg<inDataFIR_t, numCoef> shiftReg_R;
	static inDataFIR_t shiftData_L;
	static inDataFIR_t shiftData_R;

	acc_L=0;
	acc_R=0;

	Shift_Accum_Loop: for (int i = numCoef-1; i >= 0; i--) {
#pragma HLS UNROLL factor=1
//#pragma HLS LOOP_TRIPCOUNT min=1 max=numCoef avg=numCoef/2
//#pragma HLS PIPELINE

		if (i == 0) {
			shiftData_L = shiftReg_L.shift(firIn.dataCH1, i);
			shiftData_R = shiftReg_R.shift(firIn.dataCH2, i);
		} else {
			shiftData_L = shiftReg_L.read(i);
			shiftData_R = shiftReg_R.read(i);
		}

		mult_L = shiftData_L * hilbertCoef[i];
		acc_L += mult_L;
//#pragma HLS bind_storage variable=acc_L type=AddSub_DSP - ? fix type,... does this pragma change implementation?

		mult_R = shiftData_R * hilbertCoef[i];
		acc_R += mult_R;
//#pragma HLS bind_storage variable=acc_R type=AddSub_DSP

	}

	// saturate
	if (acc_L > 1) acc_L = 1;
	if (acc_R > 1) acc_R = 1;
	if (acc_L < -1) acc_L = -1;
	if (acc_R < -1) acc_R = -1;

	firOut.dataCH1 = static_cast<outDataFIR_t>(acc_L);
	firOut.dataCH2 = static_cast<outDataFIR_t>(acc_R);

	return firOut;
}


stereoData_t ssbHilbertMod::advance(stereoData_t ssbIn, ddsPhase_t freqCtrl) {
//#pragma HLS PIPELINE

	// ssbHilbertMod top-level IO data
	stereoData_t ssbOut;

	ssbInternal_t ssbData1;
	ssbInternal_t ssbData2;

	// FIR data types
	inStereoFIR_t firIn;
	outStereoFIR_t firOut;

	// delay path types (aligns with center FIR tap)
	static ap_shift_reg<data_t, centerTapDly> delay_L;
	static ap_shift_reg<data_t, centerTapDly> delay_R;
	//static data_t shiftData_L;
	//static data_t shiftData_R;

	// oscillator types
	bool reset;
	ddsPhase_t oscStep;
	cmplx_t oscOut;

	//modulator types
	//*implicit truncation of precision by type assignment - verify HW is correct)
	ssbInternal_t modulatedData;
	stereoData_t mixedData;

	// synchronize internal datapaths
	ssbData1.filterCH1 = ssbIn.dataCH1;
	ssbData1.filterCH2 = ssbIn.dataCH2;
	ssbData1.delayCH1 = ssbIn.dataCH1;
	ssbData1.delayCH2 = ssbIn.dataCH2;


	// Filter and scale input data
	firIn.dataCH1 = ssbData1.filterCH1;
	firIn.dataCH2 = ssbData1.filterCH2;

	firOut = hilbertFIR.advance(firIn);

	// repack the delayed and filtered data into single struct
	ssbData2.delayCH1 = delay_L.shift(ssbData1.delayCH1, centerTapDly-1);
	ssbData2.delayCH2 = delay_R.shift(ssbData1.delayCH2, centerTapDly-1);

	ssbData2.filterCH1 = firOut.dataCH1;
	ssbData2.filterCH2 = firOut.dataCH2;

	// run DDS oscillator to generate modulation

	oscDds.set_step(freqCtrl);
	oscOut = oscDds.advance();

	// modulate delayed and filtered data
	modulatedData.delayCH1 = oscOut.real * ssbData2.delayCH1;
	modulatedData.delayCH2 = oscOut.real * ssbData2.delayCH2;
	modulatedData.filterCH1 = -oscOut.imag * ssbData2.filterCH1;
	modulatedData.filterCH2 = -oscOut.imag * ssbData2.filterCH2;

	mixedData.dataCH1 = modulatedData.delayCH1 + modulatedData.filterCH1;
	mixedData.dataCH2 = modulatedData.delayCH2 + modulatedData.filterCH2;

	// saturate and write to output
	if (mixedData.dataCH1 > static_cast<data_t>(0.99)) {
		ssbOut.dataCH1 = static_cast<data_t>(0.99);
	} else if (mixedData.dataCH1 < static_cast<data_t>(-0.99)) {
		ssbOut.dataCH1 = static_cast<data_t>(-0.99);
	} else {
		ssbOut.dataCH1 = mixedData.dataCH1;
	}

	if (mixedData.dataCH2 > static_cast<data_t>(0.99)) {
		ssbOut.dataCH2 = static_cast<data_t>(0.99);
	} else if (mixedData.dataCH2 < static_cast<data_t>(-0.99)) {
		ssbOut.dataCH2 = static_cast<data_t>(-0.99);
	} else {
		ssbOut.dataCH2 = mixedData.dataCH2;
	}


	//std::cout << "ssbHilbertMod::advance ssbOut.dataCH1 = " << ssbOut.dataCH1 << ", ssbOut.dataCH2 = " << ssbOut.dataCH2 << std::endl;

	//fprintf(stdout,"firIn.dataCH1: %10.7f \t firOut.dataCH1: %10.7f \n", firIn.dataCH1.to_double(), firOut.dataCH1.to_double() );
	//fprintf(stdout,"ssbOut CH1: %10.7f \t mixedData CH1: %10.7f \t modulatedData dly CH1: %10.7f \t modulatedData flt CH1: %10.7f \t oscOut imag: %10.7f \t ssbData2 CH1: %10.7f \n", ssbOut.dataCH1.to_double(), mixedData.dataCH1.to_double(), modulatedData.delayCH1.to_double(), modulatedData.filterCH1.to_double(), oscOut.imag.to_double(), ssbData2.delayCH1.to_double() );
	//fprintf(stdout,"ssbOut CH2: %10.7f \t mixedData CH2: %10.7f \t modulatedData dly CH2: %10.7f \t modulatedData flt CH2: %10.7f \t oscOut real: %10.7f \t ssbData2 CH2: %10.7f \n", ssbOut.dataCH2.to_double(), mixedData.dataCH2.to_double(), modulatedData.delayCH2.to_double(), modulatedData.filterCH2.to_double(), oscOut.real.to_double(), ssbData2.filterCH2.to_double() );
	//if (i < 32) fprintf(stdout,"signal[%4d]=%10.5f \t reference[%4d]=%10.5f\n", i, signal[i].to_double(), i, reference[i].to_double() );

	return ssbOut;

}


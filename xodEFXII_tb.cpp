/*------------------------------------------------------------------------------------------------*/
/* ___::((xodEFXII_tb.cpp))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018:2023))::___
   ___::((created by eschei))___

	Purpose: testbench header for XODMK EFXII
	Device: All
	Revision History: September 1, 2017 - initial
	Revision History: May 9, 2018 - v4
	Revision History: 2022-02-09 -  XODMK revision
	Revision History: 2022-03-14 - xodDDLXN, xodMixMatrix - multi-channel Feedback Delay Network
	Revision History: 2023-07-14 - Fixed HW UI ports, xodLFOXN functionality, TB/SW CTRL functions
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <ap_int.h>

#include "AudioFile.h"

//#include "xodDDLXN.h"
//#include "xodSSBMod.h"
//#include "xodWaveshaper4.h"

#include "xodCpuCtrl.h"
#include "xodEFXII.h"
#include "xodEFXII_types.h"
#include "xodEFXII_tb.h"


// *--------------------------------------------------------------------------------* //

static std::string getFileName(const std::string &s);
static std::string getFileExt(const std::string &s);

template <class T, int nRoots>
void cyclicZn(T cZn[2][nRoots]);

static float rmsValue(int arr[], int arrSize);


/*-----------------------------------------------------------------------------*/

static std::string getFileName(const std::string &s) {
    char sep = '/';
    size_t firstIdx = s.rfind(sep, s.length());
    size_t lastIdx = s.find_last_of(".");
    size_t lenIdx = lastIdx - firstIdx - 1;
    if (firstIdx != std::string::npos) {
        return (s.substr(firstIdx + 1, lenIdx));
    }
    return("");
}

static std::string getFileExt(const std::string &s) {
    size_t firstIdx = s.find_last_of(".");
    size_t lastIdx = s.length();
    size_t lenIdx = lastIdx - firstIdx - 1;
    if (firstIdx != std::string::npos) {
        return (s.substr(firstIdx + 1, lenIdx));
    }
    return("");
}


template <class T, int nRoots>
void cyclicZn(T cZn[2][nRoots]) {

	// calculates the Zn roots of unity
	// cZn = np.zeros((n, 1)) * (0+0j)    # column vector of zero complex values

	for (int c = 0; c < nRoots; c++) {
		// z(k) = e^(((k)*2*pi*1j)/n)	# Define cyclic group Zn points
		cZn[0][c] = static_cast<T>(cos((c * 2 * pi) / nRoots));
		cZn[1][c] = static_cast<T>(sin((c * 2 * pi) / nRoots));
	}

}


// Function that Calculate Root Mean Square
// -> int arrSize = sizeof(arr)/sizeof(arr[0]);
static float rmsValue(int arr[], int arrSize)
{
    int square = 0;
    float mean, rootValue = 0.0;

    // Calculate square value.
    for (int i = 0; i < arrSize; i++) {
        square += pow(arr[i], 2);
    }

    // Calculate Mean value.
    mean = (square / (float)(arrSize));

    // Calculate Root value.
    rootValue = sqrt(mean);

    return rootValue;
}


// *--------------------------------------------------------------------------------* //
// *--------------------------------------------------------------------------------* //
// ** Test Func **

// *** FIXIT FIXIT *** -> Remove casts - use floats for tb instead of ap_fixed/ap_ufixed
static void xodEFXII_test(const std::string &efxTestName, const std::string &outDir, const int simLength,
						  const float sr, switch_t switches, lfoCtrlAll_tb lfoCTRL, ddlLengthCtrl_t ddlLengthCTRL,
						  ddlCtrlAll_t ddlCTRL, ssbModFreqCtrl_t ssbModFreqCTRL, wavshaperCtrl_t wavShaperGain,
						  float efxOutGain, AudioFile<float>* wavMain, AudioFile<float>* wavSub,
						  stereoData_t x_in[], stereoData_t y_out[])
{

	// *--------------------------------------------------------------------------------* //
	std::cout << "\n{{***** Run Test " << efxTestName << " *****}}" << std::endl;
	std::cout << std::endl;
	std::cout << "EFX Parameters:" << std::endl;
	std::cout << " - Sample Rate = " << sr << std::endl;
	// switches
	std::cout << " - efxBypass = " << static_cast<int>(switches.bypass) << std::endl;
	std::cout << " - wavShaperOn = " << switches.wavShaperOn << std::endl;
	std::cout << " - multiChan = " << switches.multiChan << std::endl;
	std::cout << " - selectCross = " << static_cast<int>(switches.selectCross) << std::endl;
	std::cout << " - selectHouse = " << static_cast<int>(switches.selectHouse) << std::endl;
	std::cout << " - selectSSB = " << static_cast<float>(switches.selectSSB) << std::endl;
	std::cout << " - lfoUpDn = " << switches.lfoUpDn << std::endl;
	std::cout << " - lfoSync = " << switches.lfoSync << std::endl;
	// ddl control
	std::cout << " - ddlCenterLock = " << ddlLengthCTRL.ddlCenterLock << std::endl;
	std::cout << " - ddlLengthBase = " << ddlLengthCTRL.ddlLengthBase << std::endl;
	std::cout << " - ddlTilt = " << ddlLengthCTRL.ddlTilt << std::endl;
	// std::cout << " - outputGain = " << static_cast<float>(ddlCTRL.outputGain) << std::endl;
	std::cout << " - wetMix = " << static_cast<float>(ddlCTRL.wetMix) << std::endl;
	std::cout << " - feedbackMix = " << static_cast<float>(ddlCTRL.feedbackMix) << std::endl;
	// ssb control
	std::cout << " - ssbCenterLock = " << ssbModFreqCTRL.ssbCenterLock << std::endl;
	std::cout << " - ssbFreqBase = " << ssbModFreqCTRL.ssbFreqBase << std::endl;
	std::cout << " - ssbTilt = " << ssbModFreqCTRL.ssbTilt << std::endl;
	// waveshaper control
	std::cout << " - wavShaperGain = " << static_cast<float>(wavShaperGain) << std::endl;
	// lfo control
	std::cout << " - lfoTargCH0 = " << static_cast<float>(lfoCTRL.lfoTargCH0) << std::endl;
	std::cout << " - lfoShapeCH0 = " << static_cast<float>(lfoCTRL.lfoShapeCH0) << std::endl;
	std::cout << " - lfoGainCH0 = " << static_cast<float>(lfoCTRL.lfoGainCH0) << std::endl;
	std::cout << " - lfoFreqCH0 = " << static_cast<float>(lfoCTRL.lfoFreqCH0) << std::endl;
	std::cout << " - lfoTargCH1 = " << static_cast<float>(lfoCTRL.lfoTargCH1) << std::endl;
	std::cout << " - lfoShapeCH1 = " << static_cast<float>(lfoCTRL.lfoShapeCH1) << std::endl;
	std::cout << " - lfoGainCH1 = " << static_cast<float>(lfoCTRL.lfoGainCH1) << std::endl;
	std::cout << " - lfoFreqCH1 = " << static_cast<float>(lfoCTRL.lfoFreqCH1) << std::endl;
	// end
	std::cout << std::endl;

	// *--------------------------------------------------------------------------------* //
	// *--------------------------------------------------------------------------------* //

	// *--------------------------------------------------------------------------------* //
	// *--- DDL Ctrl ---*

	// DDL gain controls
	// Converts a ap_fixed<n, 1> +/- 1.0 to unsigned nBits integer type
	ddlGainBus_t ddlGainBus;

	// Linear Faders:
	//ddlGainBus.uWetMix 		= ufixedToUintx_1p0<delayCtrl_t, uint16_t, 16>(ddlCTRL.wetMix);
	//ddlGainBus.uFeedbackMix = ufixedToUintx_1p0<delayCtrl_t, uint16_t, 16>(ddlCTRL.feedbackMix);

	// *** FIXIT FIXIT *** -> Remove casts - use floats for tb instead of ap_fixed/ap_ufixed

	// inverse exponential (log-like) Faders:
	// Linear Faders values -> no significant response until > 90 % .: use inverse exponential (log-like) fade
	float expFLoat_wetMix 		= invExpSqrNorm(static_cast<float>(ddlCTRL.wetMix));
	delayCtrl_t expFade_wetMix 	= static_cast<delayCtrl_t>(expFLoat_wetMix);
	ddlGainBus.uWetMix 			= ufixedToUintx_1p0<delayCtrl_t, uint16_t, 16>(ddlCTRL.wetMix);

	float expFLoat_fbMix 		= invExpSqrNorm(static_cast<float>(ddlCTRL.feedbackMix));
	delayCtrl_t expFade_fbMix 	= static_cast<delayCtrl_t>(expFLoat_fbMix);
	ddlGainBus.uFeedbackMix 	= ufixedToUintx_1p0<delayCtrl_t, uint16_t, 16>(ddlCTRL.feedbackMix);

	// currently controls Overall EFX output gain
	uint16_t outputGainBus 		= ufixedToUintx_1p0<float, uint16_t, 16>(efxOutGain);

	// ddlxnLengthCtrl
	assert(ddlLengthCTRL.ddlLengthBase + (abs(ddlLengthCTRL.ddlTilt)) <= static_cast<int>(MAXDELAY));

	//delayLen_t delayLenArray[MAXCHANNELS] {0};
//	uint32_t delayLenAggregate[3] {0};
//	ddlxnLengthCtrlAXI<static_cast<int>(MAXCHANNELS)>(ddlLengthCTRL.ddlCenterLock,
//													  ddlLengthCTRL.ddlLengthBase,
//													  ddlLengthCTRL.ddlTilt, delayLenAggregate);

	double ddlLengthArrSW[MAXCHANNELS] {0};
	ddlxnLengthCtrlSW<static_cast<int>(MAXCHANNELS)>(ddlLengthCTRL.ddlCenterLock,
			  	  	  	  	  	  	  	  	  	  	 ddlLengthCTRL.ddlLengthBase,
													 ddlLengthCTRL.ddlTilt, ddlLengthArrSW);

	uint32_t delayLenAXIBus[3] {0};
	ddlxnLengthAXIBus(ddlLengthArrSW, delayLenAXIBus);


	/////////////////////////////////////////////
	// ** Test signed <-> unsigned conversion for implementation of AXI-Lite MemoryMapped control bus
//	for (int i = 0; i < static_cast<int>(MAXCHANNELS); i++) {
//		std::cout << " - dlyLength CH[" << i << "] = " << static_cast<float>(delayLenArray[i]) << std::endl;
//	}

	//ddlLenArray_t ddlLenArrayCheck {0};
	ddlLenArray_t ddlLenFreqFixed {0};
	ddlxnLengthCtrl<delayLen_t, static_cast<int>(MAXCHANNELS)>(ddlLengthCTRL.ddlCenterLock,
															   ddlLengthCTRL.ddlLengthBase,
															   ddlLengthCTRL.ddlTilt, ddlLenFreqFixed);

	for (int c = 0; c < static_cast<int>(MAXCHANNELS); c++) {
		std::cout << "xodEFX_test: DDL Lengths cooked CH[" << c << "] = " << static_cast<float>(ddlLenFreqFixed[c])
		          << std::endl;
	}
	std::cout << std::endl;

	/////////////////////////////////////////////

	ddlLength4CHAXI_t ddlLength4CH;
	ddlLength4CH.ddlLengthCH1 = delayLenAXIBus[0];
	ddlLength4CH.ddlLengthCH2 = delayLenAXIBus[1];
	ddlLength4CH.ddlLengthCH3 = delayLenAXIBus[2];

	// DEBUG
//	for (int c = 0; c < 3; c++) {
//		std::cout << "xodEFX_test[" << c << "]: delayLenAXIBus = " << static_cast<float>(delayLenAXIBus[c])
//			 << std::endl;
//	}

	// *--------------------------------------------------------------------------------* //
	// *--- ssbModFreqCtrl ---*

	// +/-1499 Hz max/min freq ssbOsc CTRL UI (ddsCtrl_t = 32bits)
	// (-1499 * (2**36 / 48000) = -2146051992  =>  2**31 - 1 = 2147483647 (1500 overflows 32 bits)
	assert(ssbModFreqCTRL.ssbFreqBase + (abs(ssbModFreqCTRL.ssbTilt)) <= 1499);
	assert(ssbModFreqCTRL.ssbFreqBase - (abs(ssbModFreqCTRL.ssbTilt)) >= -1499);

	uint32_t ssbModFreqArray[MAXCHANNELS ] {0};
	ssbModFreqCtrlAXI<static_cast<int>(MAXCHANNELS), ddsPhaseWidth>(ssbModFreqCTRL.ssbCenterLock,
																	ssbModFreqCTRL.ssbFreqBase,
																	ssbModFreqCTRL.ssbTilt, ssbModFreqArray);

	/////////////////////////////////////////////
	// ** Test signed <-> unsigned conversion for implementation of AXI-Lite MemoryMapped control bus
//	multiDDSCtrl_t ssbModFreqArrayCheck {0};
//	multiDDSCtrl_t ssbModFreqFixed {0};
////	ssbModFreqCtrl<ddsCtrl_t, static_cast<int>(MAXCHANNELS), ddsPhaseWidth>(ssbModFreqCTRL.ssbCenterLock,
////																			ssbModFreqCTRL.ssbFreqBase,
////																			ssbModFreqCTRL.ssbTilt, ssbModFreqFixed);
//
//	for (int c = 0; c < static_cast<int>(MAXCHANNELS); c++) {
//		//ssbModFreqArrayCheck[c] = static_cast<ddsCtrl_t>(ssbModFreqArray[c]
//		ssbModFreqArrayCheck[c] = static_cast<double>(ssbModFreqArray[c] * ddsPhaseScale);
//		cout << "xodEFX_test[" << c << "]: ssbModFreqArray u32 = " << static_cast<float>(ssbModFreqArray[c])
//			 << ",    ssbModFreqArrayCheck = " << ssbModFreqArrayCheck[c] << endl;
//	}
	/////////////////////////////////////////////

	ddsFreq4CHAXI_t ssbModFreq4CH;
	ssbModFreq4CH.ddsFreqCtrlCH1 = ssbModFreqArray[0];
	ssbModFreq4CH.ddsFreqCtrlCH2 = ssbModFreqArray[1];
	ssbModFreq4CH.ddsFreqCtrlCH3 = ssbModFreqArray[2];
	ssbModFreq4CH.ddsFreqCtrlCH4 = ssbModFreqArray[3];

	// *--------------------------------------------------------------------------------* //
	// *--- Translate LFO UI controls to function port controls ---*

	lfoTypeCtrl_t 	lfoTypeCTRL_tb;
    lfoWavBus_t 	lfoWavBus2CH;

    lfoTypeCTRL_tb.lfoTargetCtrl[0] = static_cast<ap_uint<4> >(lfoCTRL.lfoTargCH0);
    lfoTypeCTRL_tb.lfoShapeCtrl[0] = static_cast<ap_uint<3> >(lfoCTRL.lfoShapeCH0);
    lfoTypeCTRL_tb.lfoTargetCtrl[1] = static_cast<ap_uint<4> >(lfoCTRL.lfoTargCH1);
    lfoTypeCTRL_tb.lfoShapeCtrl[1] = static_cast<ap_uint<3> >(lfoCTRL.lfoShapeCH1);

	lfoWavBus2CH.uLfoGainCtrlCH1 = static_cast<uint16_t>(lfoCTRL.lfoGainCH0 * lfoGainScaleUI);
	lfoWavBus2CH.uLfoGainCtrlCH2 = static_cast<uint16_t>(lfoCTRL.lfoGainCH1 * lfoGainScaleUI);

	lfoWavBus2CH.uLfoFreqCtrlCH1 = lfoFreq2phaseStep16b(sr, lfoCTRL.lfoFreqCH0);
	lfoWavBus2CH.uLfoFreqCtrlCH2 = lfoFreq2phaseStep16b(sr, lfoCTRL.lfoFreqCH1);

	std::cout << " - lfoTargCH0 = " << static_cast<float>(lfoCTRL.lfoTargCH0) << std::endl;
	std::cout << " - lfoShapeCH0 = " << static_cast<float>(lfoCTRL.lfoShapeCH0) << std::endl;

	// *--------------------------------------------------------------------------------* //
    // *--- Top-Level Func call ---*

	axis_t xAxiIn;
	axis_t yAxiOut;

	uint8_t switchBus = static_cast<uint8_t>(  static_cast<uint8_t>(switches.wavShaperOn) << 7
						 	 	 	 	 	 | static_cast<uint8_t>(switches.selectSSB) << 6
											 | static_cast<uint8_t>(switches.selectHouse) << 5
											 | static_cast<uint8_t>(switches.selectCross) << 4
											 | static_cast<uint8_t>(switches.multiChan) << 3
											 | static_cast<uint8_t>(switches.lfoUpDn) << 2
											 | static_cast<uint8_t>(switches.lfoSync) << 1
											 | static_cast<uint8_t>(switches.bypass)  );


	uint32_t axiCtrlConcat = static_cast<uint32_t>(  static_cast<uint32_t>(switchBus) << 16
						       	   	   	   	   	   | static_cast<uint32_t>(lfoTypeCTRL_tb.lfoTargetCtrl[1]) << 10
												   | static_cast<uint32_t>(lfoTypeCTRL_tb.lfoTargetCtrl[0]) << 6
												   | static_cast<uint32_t>(lfoTypeCTRL_tb.lfoShapeCtrl[1]) << 3
												   | static_cast<uint32_t>(lfoTypeCTRL_tb.lfoShapeCtrl[0])  );

	uint32_t* axiCtrl = &axiCtrlConcat;

	std::cout << "\n*** run " << efxTestName << " HLS HW function.. ***" << std::endl;

	// *--------------------------------------------------------------------------------* //
	// *--- Top-Level Simulation Loop ---*

	for (int i = 0; i < simLength; i++) {
		if (i == 0) {
			switches.lfoSync = 1;
		} else {
			switches.lfoSync = 0;
		}

		xAxiIn.write(x_in[i]);

		////////////////////////////////////////////////////////////////////////////////
		///// Top-Level Function Call /////

		xodEFXII_top(axiCtrl, ddlGainBus, ddlLength4CH, ssbModFreq4CH,
				   wavShaperGain, lfoWavBus2CH, outputGainBus, xAxiIn, yAxiOut);

		////////////////////////////////////////////////////////////////////////////////

		y_out[i] = yAxiOut.read();

		//std::cout << "xodDDL5::advance efxOut[" << i << "] = " << static_cast<float>(y_out[i].dataCH1) << ", "
		//		  << static_cast<float>(y_out[i].dataCH2) << std::endl;

		wavMain->samples[0][i] = static_cast<float>(y_out[i].dataCH1);
		wavMain->samples[1][i] = static_cast<float>(y_out[i].dataCH2);

	}
	std::cout << "\n*** HLS HW function complete ***" << std::endl;

	// *--------------------------------------------------------------------------------* //
	// *--- Write processed audio files to disk ---*

	std::string outFilePath;
	outFilePath = outDir + "xodEFX_mainOut_" + efxTestName + ".wav";
	wavMain->save(outFilePath, AudioFileFormat::Wave);
	std::cout << "\nCreated Main Output Wav: " << std::endl;
	std::cout << outFilePath << std::endl;

	// *--------------------------------------------------------------------------------* //

}


////=======================================================================
//void writeSineWaveToAudioFile()
//{
//    //---------------------------------------------------------------
//    std::cout << "**********************" << std::endl;
//    std::cout << "Running Example: Write Sine Wave To Audio File" << std::endl;
//    std::cout << "**********************" << std::endl << std::endl;
//
//    //---------------------------------------------------------------
//    // 1. Let's setup our AudioFile instance
//
//    AudioFile<float> a;
//    a.setNumChannels (2);
//    a.setNumSamplesPerChannel (48000);
//
//    //---------------------------------------------------------------
//    // 2. Create some variables to help us generate a sine wave
//
//    const float sampleRate = 48000.f;
//    const float frequencyInHz = 440.f;
//
//    //---------------------------------------------------------------
//    // 3. Write the samples to the AudioFile sample buffer
//
//    for (int i = 0; i < a.getNumSamplesPerChannel(); i++)
//    {
//        for (int channel = 0; channel < a.getNumChannels(); channel++)
//        {
//            a.samples[channel][i] = sin ((static_cast<float> (i) / sampleRate) * frequencyInHz * 2.f * M_PI);
//        }
//    }
//
//    //---------------------------------------------------------------
//    // 4. Save the AudioFile
//
//    std::string filePath = "sine-wave.wav"; // change this to somewhere useful for you
//    a.save ("sine-wave.wav", AudioFileFormat::Wave);
//}


// *--------------------------------------------------------------------------------* //

int main(int argc, char** argv)
{

	// ------------------------------------------------------------------------------------------------------------
	// XOD HLS Testbench for xodDDL4
	//
	// Arguments:
	// 1       <./BUILDGXX/xodDDL4>       		- executable (for Vitis HLS IDE this arg is implicit)
	// 2       <path/source.wav>         		- source wav file
	// 3       <path/>                          - to an output result directory (must exist)
	// ...
	// ...
	// ex. /home/eschei/XODMK/xodCode/xodHLS/audio/data/source/outkast_hornz.wav /home/eschei/XODMK/xodCode/xodHLS/audio/data/output/xodEFXII_res/
	// ex. /home/eschei/XODMK/xodCode/xodHLS/audio/data/source/focusOnMy_lp.wav /home/eschei/XODMK/xodCode/xodHLS/audio/data/output/xodEFXII_res/
	// ------------------------------------------------------------------------------------------------------------
	if(argc < 3) {
		std::cerr << "./executable <path/source.wav> <path/output.wav>" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << std::endl << "{{{***** BEGIN XODMK Audio EFX MAIN *****}}}" << std::endl;

	int ret_value;

	// *--------------------------------------------------------------------------------* //
	// *--- Process Main UI Arguments ---* //

	int         argIndex    = 1;
	std::string sourceWav   = argv[argIndex++];
    std::string outputDir   = argv[argIndex];


#ifdef WAVSRC

	// *--------------------------------------------------------------------------------* //
	// *--- load .wav file ---* //

    //---------------------------------------------------------------
    std::cout << std::endl;
    std::cout << "********************************" << std::endl;
    std::cout << "__Load .wav Source Audio File__" << std::endl;
    std::cout << "********************************" << std::endl << std::endl;

    // Create an AudioFile object and load the audio file
    AudioFile<float> wav_main;
    bool loadedOK = wav_main.load(sourceWav);
    /** check for valid audio file */
    assert (loadedOK);

    // Create an AudioFile object and load the audio file
    AudioFile<float> wav_sub;
    loadedOK = wav_sub.load(sourceWav);
    /** check for valid audio file */
    assert (loadedOK);

    std::cout << "Audio Source Properties:" << std::endl;
    std::cout << " - Bit Depth         = " << wav_main.getBitDepth()        << std::endl;
    std::cout << " - Sample Rate       = " << wav_main.getSampleRate()      << std::endl;
    std::cout << " - Num Channels      = " << wav_main.getNumChannels()     << std::endl;
    std::cout << " - Length (Seconds)  = " << wav_main.getLengthInSeconds() << std::endl;
    std::cout << std::endl;

    float sr_wav = static_cast<float>(wav_main.getSampleRate());

	// *--------------------------------------------------------------------------------* //
    // Write source audio file to disk (verify)

    std::string checkFilePath = outputDir + "xodEFX_source.wav";
    wav_main.save (checkFilePath, AudioFileFormat::Wave);


	// *--------------------------------------------------------------------------------* //
	// *--- create test data ---* //

    // *** FIXIT FIXIT *** -> Remove casts - use floats for tb instead of ap_fixed/ap_ufixed

    int tail = static_cast<int>(7 * wav_main.getSampleRate());
    const int simulationLength = wav_main.getNumSamplesPerChannel() + tail;

    stereoData_t* x_input = NULL;
    x_input = new stereoData_t[simulationLength];

    wav_main.setNumSamplesPerChannel(simulationLength);
    wav_sub.setNumSamplesPerChannel(simulationLength);

    AudioFile<float>* wavMain_ptr = &wav_main;
    AudioFile<float>* wavSub_ptr = &wav_sub;

    std::cout << "Simulation Length (.wav length + Tail) =  " << wav_main.getLengthInSeconds() << std::endl;

    for (int i = 0; i < wav_main.getNumSamplesPerChannel(); i++) {
    	assert(wav_main.samples[0][i] <=  1.0 && wav_main.samples[1][i] <=  1.0);
    	assert(wav_main.samples[0][i] >= -1.0 && wav_main.samples[1][i] >= -1.0);
    	x_input[i].dataCH1 = static_cast<data_t>(wav_main.samples[0][i]);
    	x_input[i].dataCH2 = static_cast<data_t>(wav_main.samples[1][i]);
	}

	// *--------------------------------------------------------------------------------* //
	// *--- Initialize Test Variables ---* //

    std::string testName = "TESTNAME";

    int numChan = static_cast<int>(MAXCHANNELS);

    switch_t switch_tb {0};
    switch_tb.bypass = 0;
    switch_tb.multiChan = 1;

	// *--------------------------------------------------------------------------------* //
	// *--- Initialize Outputs ---* //

    stereoData_t* y_output = NULL;
    y_output = new stereoData_t[simulationLength];

    std::string outputFilePath;

    float efxOutGain = 0.93;	// EFX Output Gain [0.0:1.0]

	// *--------------------------------------------------------------------------------* //
	// *--- Initialize LFO Parameters ---* //

    // lfoShape_t = ap_uint<3>:
    // 000 - SIN				100 - RSVD
    // 001 - SAW				101 - RSVD
    // 010 - tri				110 - RSVD
    // 011 - SQR				111 - RSVD

    // lfoTarget_t = ap_uint<4>:
    // 0000 - NONE				1000 - RSVD
    // 0001 - DDLWETMIX			1001 - RSVD
    // 0010 - DDLLENGTHBASE		1010 - RSVD
    // 0011 - SSBFREQBASE		1011 - RSVD
    // 0100 - DDLTILT			1100 - RSVD
    // 0101 - SSBTILT			1101 - RSVD
    // 0110 - DDLFEEDBACK		1110 - RSVD
    // 0111 - WAVSHAPEGAIN		1111 - RSVD


    switch_tb.lfoUpDn = 0;
    switch_tb.lfoSync = 0;

    lfoCtrlAll_tb lfoCTRL;	// currently fixed @ 2 Channels

    lfoCTRL.lfoTargCH0 = 0;
    lfoCTRL.lfoShapeCH0 = 0;
    lfoCTRL.lfoGainCH0 = 0.0;
    lfoCTRL.lfoFreqCH0 = 0.056;

    lfoCTRL.lfoTargCH1 = 0;
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1 = 0.0;
    lfoCTRL.lfoFreqCH1 = 0.056;

	// *--------------------------------------------------------------------------------* //
	// *--- Initialize Delay Parameters ---* //

	ddlLengthCtrl_t ddlLengthCTRL;
    ddlCtrlAll_t ddlCTRL;
    float delayLength = 566.6;

    switch_tb.selectCross = 0;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 566.6;
    ddlLengthCTRL.ddlTilt = 56;

	ddlCTRL.wetMix = 0.7;
	ddlCTRL.feedbackMix = 0.7;
	// ddlCTRL.outputGain = 1.0;


	// *--------------------------------------------------------------------------------* //
	// *--- Initialize Modulator Parameters ---* //

    string hilbertCoef_file = data_inputDir + "hilbertCoef.dat";

	coef_t hilbertCoef_fx[numCoef];

	float coefFloat;
	static double hilbertCoefAcc = 0;
	double hilbertBitGrowth;

	ssbModFreqCtrl_t ssbModFreqCTRL;
	ssbModFreqCTRL.ssbCenterLock = true;
	ssbModFreqCTRL.ssbFreqBase = 566.6;
	ssbModFreqCTRL.ssbTilt = 0;

	// LOAD FILTER COEFFICIENTS & calculate worst case bit-growth

	ifstream hilbertCoef(hilbertCoef_file.c_str());
	//if (hilbertCoef == NULL) {
	if (hilbertCoef.get() == 0) {
		std::cout << "Error - Cannot open input file " << hilbertCoef_file.c_str() << std::endl;
		exit(2);
	}
	std::cout << std::endl << "Opened SSB Modulator FIR coefficients file: " << hilbertCoef_file << std::endl;

	for (unsigned int i = 0; i < numCoef; i++)
	{
		hilbertCoef >> coefFloat;
		hilbertCoef_fx[i] = (coef_t)coefFloat;
		if (coefFloat < 0) {
			hilbertCoefAcc += -coefFloat;
		} else {
			hilbertCoefAcc += coefFloat;
		}
		// fprintf(stdout,"[%4d] float coef=%10.9f \t fixed coef=%10.9f \n", i, coefFloat, hilbertCoef_fx[i].to_double());
	}

	hilbertBitGrowth = static_cast<int>(ceil(log2(hilbertCoefAcc)));
	std::cout << "FIR coefficient accumulated sum = " << hilbertCoefAcc << std::endl;
	std::cout << "FIR filter worst case output bit-growth = " << hilbertBitGrowth;
	std::cout << std::endl << std::endl;


	// *--------------------------------------------------------------------------------* //
	// *--- Initialize Waveshaper ---* //

	switch_tb.wavShaperOn = 0;
	wavshaperCtrl_t wavShaperGain = 5.6;


	// *--------------------------------------------------------------------------------* //
	// *--------------------------------------------------------------------------------* //
	// *--------------------------------------------------------------------------------* //

	std::cout << std::endl << "Begin Running Tests... " << std::endl;

	// *--------------------------------------------------------------------------------* //
	// ** Test X0: Bypass - Dry Stereo Out **
#ifdef TEST0

    testName = "TEST0";

    switch_tb.bypass = 1;
    switch_tb.multiChan = 0;
    switch_tb.wavShaperOn = 0;
    switch_tb.selectCross = 0;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

    switch_tb.lfoUpDn = 0;
    switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.88;
	ddlCTRL.feedbackMix = 0.75;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 23.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = -668;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);


#endif

	// ********************************************************************************** //
	// // __Stereo (2-Ch) Tests__
	// ********************************************************************************** //

	// *--------------------------------------------------------------------------------* //
	// ** Test X1: 2CH - Center Locked  **
#ifdef TEST1

    testName = "TEST1";

    switch_tb.bypass = 0;
    switch_tb.multiChan = 0;
    switch_tb.wavShaperOn = 0;
    switch_tb.selectCross = 0;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

    switch_tb.lfoUpDn = 0;
    switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.46;
	ddlCTRL.feedbackMix = 0.77;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 86.5;		// last test 'MAYBE' -> 133.5
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = -168;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X2: 2CH - Center Locked **
#ifdef TEST2

	testName = "TEST2";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.86;
	ddlCTRL.feedbackMix = 0.33;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 7260.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = -268;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X3: 2CH - Center Locked **
#ifdef TEST3

	testName = "TEST3";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 0;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.86;
	ddlCTRL.feedbackMix = 0.33;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 260.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 69;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X3: 2CH - Center Locked **
#ifdef TEST3_ALT1

	testName = "TEST3_ALT1";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 0;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.56;
	ddlCTRL.feedbackMix = 0.86;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 260.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 69;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X4: 2CH - Center Locked **
#ifdef TEST4

	testName = "TEST4";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.86;
	ddlCTRL.feedbackMix = 0.86;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 1760.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 18;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.93;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X5: 2CH - Center Locked **
#ifdef TEST5

	testName = "TEST5";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.56;
	ddlCTRL.feedbackMix = 0.86;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 760.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = -13;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X6: 2CH - Center Locked **
#ifdef TEST6

	testName = "TEST6";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.36;
	ddlCTRL.feedbackMix = 0.86;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 3560.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 93;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X6: 2CH - Center Locked **
#ifdef TEST6_ALT1

	testName = "TEST6_ALT1";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.86;
	ddlCTRL.feedbackMix = 0.86;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 3560.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 93;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.93;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X7: 2CH - Center Locked **
#ifdef TEST7

	testName = "TEST7";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.77;
	ddlCTRL.feedbackMix = 0.77;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X8: 2CH - Center Locked **
#ifdef TEST8

	testName = "TEST8";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.77;
	ddlCTRL.feedbackMix = 0.77;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 13660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = -8;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X9: 2CH - Center Locked **
#ifdef TEST9

	testName = "TEST9";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.33;
	ddlCTRL.feedbackMix = 0.77;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 9660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 128;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X9: 2CH - Center Locked **
#ifdef TEST9_ALT1

	testName = "TEST9_ALT1";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.88;
	ddlCTRL.feedbackMix = 0.88;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 9660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 128;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.95;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X10: 2CH - Center Locked **
#ifdef TEST10

	testName = "TEST10";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.23;
	ddlCTRL.feedbackMix = 0.23;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 1760.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 18;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X11: 2CH - Center Locked **
#ifdef TEST11

	testName = "TEST11";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.56;
	ddlCTRL.feedbackMix = 0.56;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 1760.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 18;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X12: 2CH - Center Locked **
#ifdef TEST12

	testName = "TEST12";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.86;
	ddlCTRL.feedbackMix = 0.46;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 1760.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 18;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X13: 2CH - Center Locked **
#ifdef TEST13

	testName = "TEST13";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.46;
	ddlCTRL.feedbackMix = 0.86;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 1760.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 18;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X14: 2CH - Center Locked **
#ifdef TEST14

	testName = "TEST14";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.86;
	ddlCTRL.feedbackMix = 0.86;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 1760.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 18;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X15: 2CH - Center Locked **
#ifdef TEST15

	testName = "TEST15";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 1.0;
	ddlCTRL.feedbackMix = 1.0;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 1760.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 18;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// *--------------------------------------------------------------------------------* //

	// *--------------------------------------------------------------------------------* //
	// ** Test X20: Stereo (2CH) - L-R Variable Delay - cross 0 - No SSBMod **
#ifdef TEST20

	testName = "TEST20";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 0;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.6;
	ddlCTRL.feedbackMix = 0.9;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase = 1760.5 + 480.25;
    ddlLengthCTRL.ddlTilt = -480.25;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = -568;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X21: Stereo (2CH) - L-R Variable Delay - cross 0 - No SSBMod **
#ifdef TEST21

	testName = "TEST21";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 0;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.95;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase =  5040.5;
    ddlLengthCTRL.ddlTilt = 1380;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 18;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X22: Stereo (2CH) - L-R Variable Delay - cross 0 - No SSBMod **
#ifdef TEST22

	testName = "TEST22";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 0;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.93;
	ddlCTRL.feedbackMix = 0.9;

	ddlCTRL.wetMix = 0.95;
	ddlCTRL.feedbackMix = 0.93;
	ddlCTRL.outputGain = 0.93;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase = 10660.5;
    ddlLengthCTRL.ddlTilt = 2380;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X23: Stereo (2CH) - L-R Variable Delay - cross 1 - No SSBMod **
#ifdef TEST23

	testName = "TEST23";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.6;
	ddlCTRL.feedbackMix = 0.9;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase = 1760.5 + 480.25;
    ddlLengthCTRL.ddlTilt = -480.25;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = -568;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif


	// *--------------------------------------------------------------------------------* //
	// ** Test X24: Stereo (2CH) - L-R Variable Delay - cross 1 - No SSBMod **
#ifdef TEST24

	testName = "TEST24";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.95;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase = 5040.5;
    ddlLengthCTRL.ddlTilt = 1380;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 18;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif


	// *--------------------------------------------------------------------------------* //
	// ** Test X25: Stereo (2CH) - L-R Variable Delay - cross 1 - No SSBMod **
#ifdef TEST25

	testName = "TEST25";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.93;
	ddlCTRL.feedbackMix = 0.9;
	ddlCTRL.outputGain = 0.93;

	ddlCTRL.wetMix = 0.95;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase = 10660.5;
    ddlLengthCTRL.ddlTilt = 2380;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif


	// *--------------------------------------------------------------------------------* //
	// *--------------------------------------------------------------------------------* //

	// *--------------------------------------------------------------------------------* //
	// ** Test Test X70 **
#ifdef TEST70

	wavShaperOn = 1;
	wavShaperGain = 36.6;



#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test Test X71 **
#ifdef TEST71

	wavShaperOn = 1;
	wavShaperGain = 76.6;


#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test Test X72 **
#ifdef TEST72

	wavShaperOn = 1;
	wavShaperGain = 26.6;

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test Test X73 **
#ifdef TEST73

	wavShaperOn = 1;
	wavShaperGain = 46.6;

#endif


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X74 **
#ifdef TEST74

	wavShaperOn = 1;
	wavShaperGain = 156.6;


#endif


	// ********************************************************************************** //
	// // __Multi Channel (4-Ch) Tests__
	// ********************************************************************************** //

	// 4-Channel centerLocked Tilt left

	// *--------------------------------------------------------------------------------* //
	// ** Test Test X100 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X101 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X102 **


	// 4-Channel centerLocked Tilt left

	// *--------------------------------------------------------------------------------* //
	// ** Test Test X103 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X104 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X105 **




	// 4-Channel Tilt left

	// *--------------------------------------------------------------------------------* //
	// ** Test Test X106 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X107 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X108 **




	// 4-Channel Tilt right

	// *--------------------------------------------------------------------------------* //
	// ** Test Test X109 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X110 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X111 **



	// 4-Channel House centerLocked Tilt left

	// *--------------------------------------------------------------------------------* //
	// ** Test Test X112 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X113 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X114 **


	// 4-Channel House centerLocked Tilt left

	// *--------------------------------------------------------------------------------* //
	// ** Test Test X115 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X116 **


	// *--------------------------------------------------------------------------------* //
	// ** Test Test X117 **


	// 4-Channel House Tilt left


	// *--------------------------------------------------------------------------------* //
	// ** Test X118:
	// ** Multi-Chan (4CH) - ddlLocked 1 - Tilt 1 - cross 0 - SSBMod 0 - House 0 **
#ifdef TEST118

	testName = "TEST118";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 1;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 0;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase = 2640.75;
    ddlLengthCTRL.ddlTilt = -480.25;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 2.3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif


	// *--------------------------------------------------------------------------------* //
	// ** Test X119:
	// ** Multi-Chan (4CH) - ddlLocked 1 - Tilt 1 - cross 0 - SSBMod 0 - House 0 **
#ifdef TEST119

	testName = "TEST119";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 1;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 0;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase = 7660.5;
    ddlLengthCTRL.ddlTilt = 2380;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 2.3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif


	// *--------------------------------------------------------------------------------* //
	// ** Test X120:
	// ** Multi-Chan (4CH) - ddlLocked 1 - Tilt 1 - cross 0 - SSBMod 0 - House 0 **
#ifdef TEST120

	testName = "TEST120";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 1;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 0;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase = 7683;
    ddlLengthCTRL.ddlTilt = -3300;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 2.3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif


	// 4-Channel House Tilt right

	// *--------------------------------------------------------------------------------* //
	// ** Test X121:
	// ** Multi-Chan (4CH) - ddlLocked 1 - Tilt 1 - cross 0 - SSBMod 0 - House 1 **
#ifdef TEST121

	testName = "TEST121";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 1;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 0;
    switch_tb.selectHouse = 1;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase = 2640.75;
    ddlLengthCTRL.ddlTilt = -480.25;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 2.3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X122:
	// ** Multi-Chan (4CH) - ddlLocked 1 - Tilt 1 - cross 0 - SSBMod 0 - House 1 **
#ifdef TEST122

	testName = "TEST122";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 1;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 0;
    switch_tb.selectHouse = 1;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase = 7660.5;
    ddlLengthCTRL.ddlTilt = 2380;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 2.3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X123:
	// ** Multi-Chan (4CH) - ddlLocked 1 - Tilt 1 - cross 0 - SSBMod 0 - House 1 **
#ifdef TEST123

	testName = "TEST123";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 1;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 0;
    switch_tb.selectHouse = 1;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = false;
    ddlLengthCTRL.ddlLengthBase = 7683;
    ddlLengthCTRL.ddlTilt = -3300;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 2.3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 0;		// NONE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.056;	// range [0:11.7]

    lfoCTRL.lfoTargCH1  = 0;		// unused:NONE
    lfoCTRL.lfoShapeCH1 = 0;
    lfoCTRL.lfoGainCH1  = 0.0;
    lfoCTRL.lfoFreqCH1  = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// Stereo LFO SSBMod Frequency modulation tests

	// *--------------------------------------------------------------------------------* //
	// ** Test X200:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 0 - House 0 - LFO DDL Length **
#ifdef TEST200

	testName = "TEST200";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 2;		// DDLLENGTHBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.56;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.11;		// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X201:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 0 - House 0 - LFO DDL Length **
#ifdef TEST201

	testName = "TEST201";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0  = 2;		// DDLLENGTHBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.76;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.11;		// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X202:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 0 - House 0 - LFO DDL Length **
#ifdef TEST202

	testName = "TEST202";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.93;
	ddlCTRL.feedbackMix = 0.9;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 560.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 93;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

	lfoCTRL.lfoTargCH0  = 2;		// DDLLENGTHBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.36;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.36;	// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif


	// *--------------------------------------------------------------------------------* //
	// ** Test X203:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 0 - House 0 - LFO DDL Length **
#ifdef TEST203

	testName = "TEST203";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 0;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.93;
	ddlCTRL.feedbackMix = 0.9;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 560.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 93;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

	lfoCTRL.lfoTargCH0  = 2;		// DDLLENGTHBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0  = 0.86;		// range [0:1]
    lfoCTRL.lfoFreqCH0  = 0.36;	// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif


	// *--------------------------------------------------------------------------------* //
	// ** Test X210:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 1 - House 0 - LFO SSB Freq **
#ifdef TEST210

	testName = "TEST210";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0 = 3;			// SSBFREQBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0 = 0.0;		// range [0:1]
    lfoCTRL.lfoFreqCH0 = 0.56;		// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X211:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 1 - House 0 - LFO SSB Freq **
#ifdef TEST211

	testName = "TEST211";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0 = 3;			// SSBFREQBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0 = 0.07;		// range [0:1]
    lfoCTRL.lfoFreqCH0 = 0.03;		// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 0.056;	// range [0:11.7]


	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X212:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 1 - House 0 - LFO SSB Freq **
#ifdef TEST212

	testName = "TEST212";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0 = 3;			// SSBFREQBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0 = 0.23;		// range [0:1]
    lfoCTRL.lfoFreqCH0 = 0.03;		// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 0.056;	// range [0:11.7]

	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X213:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 1 - House 0 - LFO SSB Freq **
#ifdef TEST213

	testName = "TEST213";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0 = 3;			// SSBFREQBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0 = 0.56;		// range [0:1]
    lfoCTRL.lfoFreqCH0 = 0.03;		// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 0.056;	// range [0:11.7]

	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X214:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 1 - House 0 - LFO SSB Freq **
#ifdef TEST214

	testName = "TEST214";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0 = 3;			// SSBFREQBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0 = 0.07;		// range [0:1]
    lfoCTRL.lfoFreqCH0 = 0.16;		// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 1.6;

	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X215:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 1 - House 0 - LFO SSB Freq **
#ifdef TEST215

	testName = "TEST215";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0 = 3;			// SSBFREQBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0 = 0.26;		// range [0:1]
    lfoCTRL.lfoFreqCH0 = 0.16;		// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 1.6;

	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	// *--------------------------------------------------------------------------------* //
	// ** Test X216:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 1 - House 0 - LFO SSB Freq **
#ifdef TEST216

	testName = "TEST216";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0 = 3;			// SSBFREQBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0 = 0.56;		// range [0:1]
    lfoCTRL.lfoFreqCH0 = 0.16;		// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 1.6;

	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif


	// *--------------------------------------------------------------------------------* //
	// ** Test X217:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 1 - House 0 - LFO SSB Freq **
#ifdef TEST217

	testName = "TEST217";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0 = 3;			// SSBFREQBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0 = 0.16;		// range [0:1]
    lfoCTRL.lfoFreqCH0 = 0.46;		// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 1.6;

	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif


	// *--------------------------------------------------------------------------------* //
	// ** Test X218:
	// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 1 - House 0 - LFO SSB Freq **
#ifdef TEST218

	testName = "TEST218";

	switch_tb.bypass = 0;
	switch_tb.multiChan = 0;
	switch_tb.wavShaperOn = 0;
	switch_tb.selectCross = 1;
    switch_tb.selectHouse = 0;
    switch_tb.selectSSB = 1;

	switch_tb.lfoUpDn = 0;
	switch_tb.lfoSync = 0;

	ddlCTRL.wetMix = 0.9;
	ddlCTRL.feedbackMix = 0.93;

    ddlLengthCTRL.ddlCenterLock = true;
    ddlLengthCTRL.ddlLengthBase = 5660.5;
    ddlLengthCTRL.ddlTilt = 0;

    ssbModFreqCTRL.ssbCenterLock = true;
    ssbModFreqCTRL.ssbFreqBase = 3;
    ssbModFreqCTRL.ssbTilt = 0;

	wavShaperGain = 56.6;

	efxOutGain = 0.99;

    lfoCTRL.lfoTargCH0 = 3;			// SSBFREQBASE
    lfoCTRL.lfoShapeCH0 = 0;		// sin
    lfoCTRL.lfoGainCH0 = 0.36;		// range [0:1]
    lfoCTRL.lfoFreqCH0 = 0.46;		// range [0:11.7]

    //lfoCTRL.lfoTargCH1 = 0;		// unused:NONE
    //lfoCTRL.lfoShapeCH1 = 0;
    //lfoCTRL.lfoGainCH1 = 0.0;
    //lfoCTRL.lfoFreqCH1 = 1.6;

	xodEFXII_test(testName, outputDir, simulationLength, sr_wav, switch_tb, lfoCTRL,
				  ddlLengthCTRL, ddlCTRL, ssbModFreqCTRL, wavShaperGain,
				  efxOutGain, wavMain_ptr, wavSub_ptr, x_input, y_output);

#endif

	std::cout << "** Test Complete **" << std::endl;
	// *--------------------------------------------------------------------------------* //

	std::cout << std::endl;

    delete [] x_input;
    delete [] y_output;



#else
	// *--------------------------------------------------------------------------------* //
	// *--- generate test signal ---* //

	float F_1 = 600.0;
	float A_1 = 3.0;

	float F_2 = 920.0;
	float A_2 = 1.0;

	float F_3 = 1240.0;
	float A_3 = 2.0;

	float t[WINDOW_LENGTH];
	for (int i=0; i<WINDOW_LENGTH; i++) {
		t[i] = i/sampleRate;
	}

	float x_flt, x_max, x_scale, x_win;
	float win_array[WINDOW_LENGTH];
	float xFlt[WINDOW_LENGTH];
	data_t x[WINDOW_LENGTH];
	data_t xWin[WINDOW_LENGTH];
	for (int i=0; i<WINDOW_LENGTH; i++) {

		// mixed sinusoid signal
		x_flt = A_1 * sin(2*pi*F_1*t[i]) + A_2*sin(2*pi*F_2*t[i]) + A_3*sin(2*pi*F_3*t[i]);
		if (i==0) {
			x_max = x_flt;
			xFlt[i] = x_flt;
		} else {
			if (x_flt > x_max) {
				x_max = x_flt;
			}
			xFlt[i] = x_flt;
		}
	}

	x_scale = 1/x_max;

	for (int i=0; i<WINDOW_LENGTH; i++) {
		// Hamming window
		x_win = 0.54f - 0.46f * cos(2.0f * M_PI * i / (float)(WINDOW_LENGTH - 1));

		win_array[i] = x_win;
		x[i] = (data_t)xFlt[i];

		xWin[i] = (data_t)(xFlt[i]*x_win*x_scale);
	}

	// *--------------------------------------------------------------------------------* //
	// *--- create test data ---* //

	stereoData_t x_input[TEST_LENGTH * 10];

	unsigned int h = 0;
	for (int i=0; i<TEST_LENGTH * 10; i++) {
		if (i%10==0) {
			if (h<WINDOW_LENGTH) {
				x_input[i].dataCH1 = xWin[h];
				x_input[i].dataCH2 = xWin[h];
				h++;
			}
		} else {
			x_input[i].dataCH1 = (data_t)0;
			x_input[i].dataCH2 = (data_t)0;
		}
	}


	// *--------------------------------------------------------------------------------* //
	// *--- Initialize Delay Parameters ---* //

	stereoData_t fbInit;
	fbInit.dataCH1 = 0;
	fbInit.dataCH2 = 0;

	ap_uint<1> setDlengthFlag = 0;
	delay_t dlyLength = 560.5;
	ap_uint<1> selectExtFB = 0;
	ctrl_t wetMix = 0.7;
	ctrl_t feedbackGain = 0.5;
	ap_uint<1> dinValid = 0;

	int ret_value;


	// *--------------------------------------------------------------------------------* //
	// *--- run top-level function ---* //



	stereoData_t fb_output[TEST_LENGTH * 10];
	stereoData_t y_output[TEST_LENGTH * 10];

	FILE *fp4, *fp5, *fp6, *fp7;


	j = 0;
	for (i=0; i<TEST_LENGTH * 10; i++) {
		if (i==0) {
			setDlengthFlag = 1;
			dinValid = 1;
			//dinTest = x_input[i];
			xodDDL4_top(setDlengthFlag, dlyLength, selectExtFB, feedbackGain, wetMix, dinValid, x_input[i], fbInit, fb_output[i], y_output[i]);
		} else {
			if (i%10==0) {
				j++;
				dinValid = 1;
				//dinTest = x_input[i];
			} else {
				dinValid = 0;
			}
			setDlengthFlag = 0;
			xodDDL4_top(setDlengthFlag, dlyLength, selectExtFB, feedbackGain, wetMix, dinValid, x_input[i], y_output[i-1], fb_output[i], y_output[i]);
		}
	}

	// WRITE OUTPUT RESULTS

	string inSrcL = ver_outputDir + "in_src_L.dat";
	string inSrcR = ver_outputDir + "in_src_R.dat";
	string outDlyL = ver_outputDir + "out_dly_L.dat";
	string outDlyR = ver_outputDir + "out_dly_R.dat";

	fp4=fopen(inSrcL.c_str(),"w");
	fp5=fopen(inSrcR.c_str(),"w");
	fp6=fopen(outDlyL.c_str(),"w");
	fp7=fopen(outDlyR.c_str(),"w");

/*	fp4=fopen("C:/odmkDev/odmkCode/odmkHLS/audio/effx/odmkDDL3/data/in_src_L.dat","w");
	fp5=fopen("C:/odmkDev/odmkCode/odmkHLS/audio/effx/odmkDDL3/data/in_src_R.dat","w");
	fp6=fopen("C:/odmkDev/odmkCode/odmkHLS/audio/effx/odmkDDL3/data/out_dly_L.dat","w");
	fp7=fopen("C:/odmkDev/odmkCode/odmkHLS/audio/effx/odmkDDL3/data/out_dly_R.dat","w");*/

	k = 0;
	for (i=0; i<TEST_LENGTH * 10; i++)
	{
		if (i%10==0) {
			fprintf(fp4,"%10.7f\n", x_input[i].dataCH1.to_double());
			fprintf(fp5,"%10.7f\n", x_input[i].dataCH2.to_double());
			fprintf(fp6,"%10.7f\n", y_output[i].dataCH1.to_double());
			fprintf(fp7,"%10.7f\n", y_output[i].dataCH2.to_double());
		}
	}
	fclose(fp4);
	fclose(fp5);
	fclose(fp6);
	fclose(fp7);

//	// CHECK RESULTS ** using ap_shift_reg introduces extra cycle of latency that needs to be corrected for, hence i+1's
//	for (i=0;i<nSamples-1;i++)
//	{
//		diff_L = y_output[i+1].dataCH1.to_double() - y_reference[i].dataCH1.to_double();
//		diff_R = y_output[i+1].dataCH2.to_double() - y_reference[i].dataCH2.to_double();
//		if (i<64) {
//			fprintf(stdout,"CH1 output[%4d]=%10.7f \t CH1 reference[%4d]=%10.7f \t CH2 output[%4d]=%10.7f \t CH2 reference[%4d]=%10.7f \n", i+1, y_output[i+1].dataCH1.to_double(), i, y_reference[i].dataCH1.to_double(), i+1, y_output[i+1].dataCH2.to_double(), i, y_reference[i].dataCH2.to_double() );
//		}
//		diff_L = fabs(diff_L);
//		diff_R = fabs(diff_R);
//		tot_diff_L += diff_L;
//		tot_diff_R += diff_R;
//	}
//	fprintf(stdout, "CH1 TOTAL ERROR =%f, CH2 TOTAL ERROR =%f\n",tot_diff_L, tot_diff_R);
//
//	if (tot_diff_L < 1.0 || tot_diff_R < 1.0) {
//		fprintf(stdout, "\nTEST PASSED!\n");
//		ret_value =0;
//	} else {
//		fprintf(stdout, "\nTEST FAILED!\n");
//		ret_value =1;
//	}
//	return ret_value;


#endif

	ret_value = 0;

	return ret_value;


}

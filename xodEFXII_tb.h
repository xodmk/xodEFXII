/*------------------------------------------------------------------------------------------------*/
/* ___::((xodEFXII_tb.h))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018:2023))::___

	Purpose: testbench header for XODMK Zynq EFXII
	Device: All
	Revision History: September 1, 2017 - initial
	Revision History: May 9, 2018 - v4
	Revision History: 2022-01-21 -  v4 xodmk
	Revision History: 2023-09-09 - Added test variations for UI CTRL
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
 *---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#ifndef __XODEFXII_TB_H__
#define __XODEFXII_TB_H__

#include <string>


// *--------------------------------------------------------* //

/* USER SETTINGS */

// define to load .wav file, comment out for test signal
#define WAVSRC


// Stereo 2-Channel Delay Tests

// ** Bypass - Dry Stereo Out **
//#define TEST0

// ** Stereo (2CH) - Center Locked - Demo Tests **
//#define TEST1
//#define TEST2
//#define TEST3
//#define TEST3_ALT1
//#define TEST4
//#define TEST5
//#define TEST6
//#define TEST6_ALT1
//#define TEST7
//#define TEST8
//#define TEST9
//#define TEST9_ALT1


//#define TEST10		// TEST4 with gain = 1.0, wetmix = 0, ddlfbmix = 0
//#define TEST11		// TEST4 with gain = 1.0, wetmix = 0.5, ddlfbmix = 0
//#define TEST12		// TEST4 with gain = 1.0, wetmix = 1.0, ddlfbmix = 0
//#define TEST13		// TEST4 with gain = 1.0, wetmix = 0, ddlfbmix = 7.7
//#define TEST14		// TEST4 with gain = 1.0, wetmix = 0.5, ddlfbmix = 7.7
//#define TEST15		// TEST4 with gain = 1.0, wetmix = 1.0, ddlfbmix = 7.7

// ** Stereo (2CH) - L-R Variable Delay - cross 0 - No SSBMod **
//#define TEST20
//#define TEST21
//#define TEST22

// ** Stereo (2CH) - L-R Variable Delay - cross 1 - No SSBMod **
//#define TEST23
//#define TEST24
//#define TEST25


// FIXIT ** Stereo (2CH) - Center Locked - cross 0 - SSBMOD Tilt **
/*
#define TEST30
#define TEST31
#define TEST32
#define TEST33
*/

// FIXIT ** Stereo (2CH) - Center Locked - cross 1 - SSBMOD Tilt **
/*
#define TEST34
#define TEST35
#define TEST36
#define TEST37
*/

// FIXIT ** Stereo (2CH) - L-R Variable Delay - cross 0 - SSBMOD Tilt **
/*
#define TEST38
#define TEST39
#define TEST40
#define TEST41
*/

// FIXIT ** Stereo (2CH) - L-R Variable Delay - cross 1 - SSBMOD Tilt **
/*
#define TEST38
#define TEST39
#define TEST40
#define TEST41
*/



// __Multi Channel (4-Ch) Tests__

/*

// FIXIT ** Multi-Chan (4CH) - ddlLocked 1 - Tilt left - cross 0 - SSBMod 0 - House 0 **
#define TEST100
#define TEST101
#define TEST102

// FIXIT ** Multi-Chan (4CH) - ddlLocked 1 - Tilt right - cross 0 - SSBMod 0 - House 0 **
#define TEST103
#define TEST104
#define TEST105


// FIXIT ** Multi-Chan (4CH) - ddlLocked 1 - Tilt left - cross 0 - SSBMod 0 - House 0 **
#define TEST106
#define TEST107
#define TEST108

// FIXIT ** Multi-Chan (4CH) - ddlLocked 1 - Tilt right - cross 0 - SSBMod 0 - House 0 **
#define TEST109
#define TEST110
#define TEST111



// FIXIT ** Multi-Chan (4CH) - ddlLocked 1 - Tilt left - House 1 **
//#define TEST112
//#define TEST113
//#define TEST114

// FIXIT ** Multi-Chan (4CH) - ddlLocked 1 - Tilt left - House 1 **
//#define TEST115
//#define TEST116
//#define TEST117

*/

// ** Multi-Chan (4CH) - Locked 1 - Tilt 1 - cross 0 - SSBMod 0 - House 0 **
//#define TEST118
//#define TEST119
//#define TEST120

// ** Multi-Chan (4CH) - Locked 1 - Tilt 1 - cross 0 - SSBMod 0 - House 1 **
//#define TEST121
//#define TEST122
//#define TEST123


// __LFO Tests - target DDLLENGTH__
// ** Multi-Chan 0 - ddlLocked 1 - Tilt 0 - cross 1 - SSBMod 1 - House 0 - LFO DDL Length **

#define TEST200
#define TEST201
#define TEST202
#define TEST203

// __LFO Tests - target SSBMODFREQ__

//#define TEST210
//#define TEST211
//#define TEST212
//#define TEST213
//#define TEST214
//#define TEST215
//#define TEST216
//#define TEST217
//#define TEST218

// __Waveshaper Tests__

// FIXIT ** Waveshaper Tests **
//#define TEST70
//#define TEST71
//#define TEST72
//#define TEST73
//#define TEST74



//const string outputDir = "C:/odmkDev/odmkCode/odmkHLS/audio/effx/odmkDDL4/";
//const string verify_subdir = "data/";
//const string ver_outputDir = outputDir + verify_subdir;

const string data_inputDir = "/home/eschei/xodmk/xodCode/xodHLS/audio/data/";


#define WINDOW_LENGTH 256

#define TEST_LENGTH 1000

const float sampleRate = 48000.0;
unsigned int nSamples = 48000;


// *--- Constants ---* //
// const float pi = 3.1415926536;


// *--------------------------------------------------------* //
// *--- typedefs tb ---* //

// simplifies external UI programming -> converted @ top-level func call
struct lfoCtrlAll_tb {
	// channel 0
	uint8_t lfoTargCH0;		// 4-bit code - ap_uint<4>
	uint8_t lfoShapeCH0;	// 3-bit code - ap_uint<3>
	float lfoGainCH0;		// range [0:1]
	float lfoFreqCH0;		// range [?:?]
	// channel 1
	uint8_t lfoTargCH1;		// 4-bit code - ap_uint<4>
	uint8_t lfoShapeCH1;	// 3-bit code - ap_uint<3>
	float lfoGainCH1;		// range [0:1]
	float lfoFreqCH1;		// range [?:?]
};


// *--------------------------------------------------------* //

static void xodEFXII_test(const std::string &efxTestName, const std::string &outDir, const int simLength,
			  switch_t switches, lfoCtrlAll_tb lfoCTRL, ddlLengthCtrl_t ddlLengthCTRL,
			  ddlCtrlAll_t ddlCTRL, ssbModFreqCtrl_t ssbModFreqCTRL, wavshaperCtrl_t wavShaperGain,
			  float efxOutGain, AudioFile<float>* wavMain, AudioFile<float>* wavSub,
			  stereoData_t x_in[], stereoData_t y_out[]);

// *--------------------------------------------------------* //


#endif

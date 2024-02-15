/*------------------------------------------------------------------------------------------------*/
/* ___::((xodEFXII_types.h))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2018))::___
   ___::((created by eschei))___

	Purpose: type definition header for XODMK EFXII
	Device: All
	Revision History: 2022-09-22 -  create
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#ifndef __XODEFXIITYPES_H__
#define __XODEFXIITYPES_H__


#include <math.h>
#include <stdint.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>


/*------------------------------------------------------------------------------------------------*/

#define PROBE_LOW 560
#define PROBE_HIGH 590

/*------------------------------------------------------------------------------------------------*/

#define MAXCHANNELS 4
#define MAXDELAY 16384

#define LFOCHANNELS 2

const float pi = 3.1415926536;

const double Fclk = 100000000;			// 100 MHz (This clk should match the actual HW clock)
const double sr = 48000;				// audio sample rate


const unsigned int dataWidth = 24;
const unsigned int extTop = 3;
const unsigned int extBottom = 1;
const unsigned int tDataWidth = 48;


typedef ap_fixed<dataWidth, 2> data_t;
//typedef ap_fixed<dataWidth+8, 6> dataExt_t;
typedef ap_fixed<dataWidth + extTop + extBottom, 2 + extTop> dataExt_t;


//typedef ap_fixed<2 * dataWidth, dataWidth - 2> dataConv_t;
//typedef ap_ufixed<dataWidth, dataWidth> dataUfix_t;

// aggregated channels should match with audio interface
// Zedboard xodZedADAU1761axi (VHDL: audioIO_in_tdata <= line_in_l & line_in_r;)
struct stereoData_t {
	data_t dataCH2;
	data_t dataCH1;
};

struct stereoDataExt_t {
	dataExt_t dataCH2;
	dataExt_t dataCH1;
};

typedef hls::stream<stereoData_t> axis_t;

using multiData_t = dataExt_t [MAXCHANNELS];

const ap_ufixed<16, 1> channelScaling = static_cast<ap_ufixed<16, 1> >(2.0 / MAXCHANNELS);
const float houseScaling = -2.0 / MAXCHANNELS;

// T scalingFactor = std::sqrt(1.0/size);
const float hadamardScaling = sqrt(1.0 / static_cast<float>(MAXCHANNELS));

//typedef ap_uint<tDataWidth> axisData_t;
//typedef hls::axis<stereoData_t, 0, 0, 0> axis_t;


struct switch_t {
	ap_uint<1> bypass;
	ap_uint<1> filterOn;
	ap_uint<1> wavShaperOn;
	ap_uint<1> multiChan;
	ap_uint<1> selectCross;
	ap_uint<1> selectHouse;
	ap_uint<1> selectSSB;
	ap_uint<1> reserve;
};

struct switchLFO_t {
	ap_uint<1> lfoUpDn;
	ap_uint<1> lfoSync;
};


/*------------------------------------------------------------------------------------------------*/
// *--- DDL Types

//const unsigned DLYBWIDTH = ceil(log2(MAXDELAY));
const unsigned DLYBWIDTH = 16;
const unsigned DLYFRACWIDTH = 8;

typedef ap_ufixed<16, 1> delayCtrl_t;
typedef ap_ufixed<18, 1> delayGain_t;
typedef ap_uint<DLYBWIDTH> delayLenInt_t;
typedef ap_ufixed<DLYBWIDTH+DLYFRACWIDTH, DLYBWIDTH> delayLen_t;	// ap_ufixed<24, 16>
using ddlLenArray_t = delayLen_t [MAXCHANNELS];

typedef ap_ufixed<DLYFRACWIDTH, 0> delayFrac_t;
typedef ap_ufixed<32, 16> delayMath_t;

struct ddlLength4CH_t {
	delayLen_t ddlLengthCH1;
	delayLen_t ddlLengthCH2;
	delayLen_t ddlLengthCH3;
	delayLen_t ddlLengthCH4;
};

struct ddlLength4CHAXI_t {
	uint32_t ddlLengthCH1;
	uint32_t ddlLengthCH2;
	uint32_t ddlLengthCH3;
};

struct ddlLengthCtrl_t {
	bool ddlCenterLock;
	double ddlLengthBase;
	double ddlTilt;
};

struct ddlGainCtrl_t {
	delayCtrl_t wetMix;
	delayCtrl_t feedbackMix;
};

struct ddlGainBus_t {
	uint16_t uWetMix;
	uint16_t uFeedbackMix;
};


struct ddlCtrlAll_t {
	delayCtrl_t wetMix;
	delayCtrl_t feedbackMix;
};


const float ddlFDNBaseMs = 150;				// minimum delay used to calculate channel delays

//double decayGain = 0.85;				// ? fixed delay gain ? test using variable decay..

const float ddlFDNChanBase = ddlFDNBaseMs * 0.001 * sr;
//const delayMath_t ddlFDNChanBase = static_cast<delayMath_t>(ddlFDNBaseMs * 0.001 * Fs);


// *-----------------------------------------------------------------------------------* //
///// Single Side-band Modulator types /////////////////////

//typedef ap_fixed<dataWidth, 2> ssbData_t;
//typedef ap_fixed<dataWidth+8, 6> ssbData_t;
typedef ap_fixed<dataWidth + extTop + extBottom, 2 + extTop> ssbData_t;

// ** same as data_t

struct ssbModFreqCtrl_t {
	bool ssbCenterLock;
	double ssbFreqBase;
	double ssbTilt;
};


// *-----------------------------------------------------------------------------------* //
///// DDS Typedefs /////////////////////

// ** copied from xodSSBMod.h

//const double Fs = 100000000;	// FPGA clock frequency

const int ddsDataWidth = 24;

// *** limitation for 32 bit UI frequency control ***
// For Fclk = 100MHz internal clock step - swap 100MHz Fclk for 48000 Fs when calculating step (SW func)
// (-1499 * (2**36 / 48000) = -2146051992  =>  2**31 - 1 = 2147483647 (1500 overflows 32 bits)
// +/-1499 Hz = max/min freq for 32 bit CTRL UI
// (or - FIXIT - implement DDS freq equation -> requires 'power of 2' in FPGA)
const int ddsControlWidth = 32;
const int ddsPhaseWidth = 36;									// high resolution phase accumulator width

// phaseScale: scale lower rez freq control (step value) to full accumulator width
//const int ddsPhaseScale = pow(2, ddsPhaseWidth - ddsControlWidth);
// *** ERROR: [HLS 207-2607] non-type template argument is not a constant expression
const int ddsPhaseScale = 16;	// 2**(36-32)


const int lutAddrWidth = 7;						    			// sincos LUT addr width
const int taylorOrder = 1;

typedef ap_fixed<ddsControlWidth, ddsControlWidth> ddsCtrl_t;	// DDS control data type
using multiDDSCtrl_t = ddsCtrl_t [MAXCHANNELS];
typedef ap_fixed<ddsPhaseWidth, ddsPhaseWidth> ddsPhase_t;		// phase increment type
typedef ap_fixed<ddsDataWidth, 2> ddsData_t;

//using ddsMultiPhase_t = ddsPhase_t [NCHANNELS];
//using ddsMultiData_t = pair<ddsData_t, ddsData_t> [NCHANNELS];

struct cmplx_t {
	ddsData_t real;
	ddsData_t imag;
};

struct ddsFreq4CH_t {
	ddsCtrl_t ddsFreqCtrlCH1;
	ddsCtrl_t ddsFreqCtrlCH2;
	ddsCtrl_t ddsFreqCtrlCH3;
	ddsCtrl_t ddsFreqCtrlCH4;
};

struct ddsFreq4CHAXI_t {
	uint32_t ddsFreqCtrlCH1;
	uint32_t ddsFreqCtrlCH2;
	uint32_t ddsFreqCtrlCH3;
	uint32_t ddsFreqCtrlCH4;
};


//typedef ap_uint<128> ddsFreq4CH_t;


// *-----------------------------------------------------------------------------------* //
///// FILTER Typedefs /////////////////////

const unsigned int fltCtrlWidth = 24;

typedef ap_fixed<fltCtrlWidth, 3> fltCtrl_t;		// 3 required for bit-growth at high-rez
											// control type for G, 0.0 < G < 1.0 (from simulation)


// FILTER TYPES:
// 0 = MLHP, 1 = ML4P, 2 = SEMLP, 3 = SEMHP, 4 = SEMBP, 5 = SEMBS
typedef ap_uint<3> filterType_t;

struct fParam_t {
	filterType_t type;
	fltCtrl_t alpha0;
	fltCtrl_t rho;
	fltCtrl_t beta1;
	fltCtrl_t beta2;
	fltCtrl_t beta3;
	fltCtrl_t beta4;
};


struct moogL_t {
	stereoData_t xn;
	stereoData_t fb_LP1;
	stereoData_t fb_LP2;
	stereoData_t fb_LP3;
	stereoData_t fb_LP4;
	stereoData_t yn;
};


// Processor CTRL tytpes
struct fParamU_t {
	// filterType_t type;
	float K;
	float G;
	float alpha0;
	float beta1;
	float beta2;
	float beta3;
	float beta4;
};

struct stereoDataRef_t {
	float dataL;
	float dataR;
};


// *-----------------------------------------------------------------------------------* //
///// LFO Typedefs /////////////////////

const int lfoDataWidth 		= 16;			// DDS PW-LFO output width
const int lfoFreqCtrlWidth 	= 16;			// LFO Frequency Control bit-width
const int lfoGainWidth 		= 16;			// LFO Gain Control bit-width
const int lfoGainScaleUI 	= 512;			// ** approx: ~(MAXDELAY / 2) / MAXLFOFREQ
											// ** current: (16384 / 2) / 11.7 = ~700 => ~512
const int lfoPhaseWidth 	= 28;			// low resolution phase accumulator width


// phaseScale: DDS phaseScale vs. LFO phaseScale
//
// phaseScale: scale dds freq control (step value) to full accumulator width
// Since dds FreqCtrlWidth is less than dds PhaseWidth, we want to control frequency range selection
// by scaling / un-scaling -> effectively slides the FreqCtrlWidth bits window to capture desired freq range
//
//const int ddsPhaseScale = pow(2, lfoPhaseWidth - lfoFreqCtrlWidth);
// *** ERROR: [HLS 207-2607] non-type template argument is not a constant expression
// FIXIT -> set to constant for now..
// const int ddsPhaseScale = 4096;	// 2**(28-16)
//
// For LFO, Scale can be 1 (no scaling), because the desired freq range is captured in the LSBs
// * lfoPhaseScale shifts usable range of Accumulator width to allow for higher output freqs when necessary
const int lfoPhaseScale 	= 1;	// For LFO, we want to focus on lowest sub-range


const int lfoLutAddrWidth 	= 7;	// sincos LUT addr width
//const int lfoTaylorOrder 	= 1;

typedef ap_uint<lfoPhaseWidth> lfoPhase_t;						// low resolution phase increment type

typedef ap_fixed<lfoDataWidth, 2> lfoData_t;					// DDS LFO output
using multiLfoData_t = lfoData_t [LFOCHANNELS];

struct lfoCmplx_t {
	lfoData_t real;
	lfoData_t imag;
};

typedef ap_ufixed<lfoGainWidth, lfoGainWidth> lfoGain_t;	// LFO gain control data type
using multiLfoGain_t = lfoGain_t [LFOCHANNELS];

typedef ap_fixed<lfoFreqCtrlWidth, lfoFreqCtrlWidth> lfoFreq_t;	// LFO (DDS) freq control data type
using multiLfoFreq_t = lfoFreq_t [LFOCHANNELS];

// lfoShape_t:
// 000 - SIN				100 - RSVD
// 001 - SAW				101 - RSVD
// 010 - tri				110 - RSVD
// 011 - SQR				111 - RSVD
typedef ap_uint<3> lfoShape_t;
using multiLfoShape_t = lfoShape_t [LFOCHANNELS];


// lfoTarget_t:
// 0000 - NONE				1000 - RSVD
// 0001 - DDLWETMIX			1001 - RSVD
// 0010 - DDLLENGTHBASE		1010 - RSVD
// 0011 - SSBFREQBASE		1011 - RSVD
// 0100 - DDLTILT			1100 - RSVD
// 0101 - SSBTILT			1101 - RSVD
// 0110 - DDLFEEDBACK		1110 - RSVD
// 0111 - WAVSHAPEGAIN		1111 - RSVD
typedef ap_uint<4> lfoTarget_t;
using multiLfoTarget_t = lfoTarget_t [LFOCHANNELS];

// LFO UI Control ports:
struct lfoTypeCtrl_t {
	multiLfoTarget_t lfoTargetCtrl;		// 4-bits x nChannels
	multiLfoShape_t lfoShapeCtrl;		// 3-bits x nChannels
};

struct lfoWavCtrl_t {
	multiLfoGain_t lfoGainCtrl;			// 16-bits x nChannels (lfoGainWidth)
	multiLfoFreq_t lfoFreqCtrl;			// 16-bits x nChannels (lfoFreqCtrlWidth)
};

// Fixed to 2 Channel x 32bit boundaries to match GPIO data bus
struct lfoWavBus_t {
	uint16_t uLfoGainCtrlCH1;
	uint16_t uLfoFreqCtrlCH1;
	uint16_t uLfoGainCtrlCH2;
	uint16_t uLfoFreqCtrlCH2;
};


// *-----------------------------------------------------------------------------------* //
///// wavshaper types /////////////////////

typedef ap_ufixed<16,10>  wavshaperCtrl_t;


// *-----------------------------------------------------------------------------------* //
///// filter data types /////////////////////


// **assume using DSP48e1 18x25 Mpy**

const unsigned int firDataWidth = dataWidth;
const unsigned int coefWidth = 18;
const unsigned int firBitGrowth = 2;			// from pythonFIR.py

typedef ap_fixed<coefWidth, 2>	coef_t;
typedef ap_ufixed<coefWidth, 2>  gain_t;

//typedef ap_fixed<firDataWidth, 2>	firData_t;
//typedef ap_fixed<firDataWidth+coefWidth, 2 + firBitGrowth>	acc_t;

// ** Using extended data types **
//typedef ap_fixed<firDataWidth+8, 6>	firData_t;
//typedef ap_fixed<firDataWidth+coefWidth+8, 6 + firBitGrowth>	acc_t;
typedef ap_fixed<firDataWidth + extTop + extBottom, 2 + extTop>	firData_t;
typedef ap_fixed<firDataWidth + coefWidth + extTop + extBottom, 2 + extTop + firBitGrowth>	acc_t;


// 369 tap Hilbert FIR - designed in Python
const unsigned int numCoef = 369;				// should match pythonFIR.py parameters
const unsigned int centerTapDly = 184;			// position of center FIR Coefficient
//const gain_t firGain = 2.63448870847;
//const gain_t firGainInv = 0.37958029456909603;


// *-----------------------------------------------------------------------------------* //
///// hilbert FIR coefficient /////////////////////


/* FIR Coefficients generated using Python+Scipy (ssbHilbertMod.py) :
 * fHilbert=sp.signal.remez(99, [0.03, 0.47],[1], type='hilbert'); */

// Example - current design reads coefficients from .dat file
const coef_t hilbertCoef[numCoef] = {
		-0.0000027971672683,    0.0005668243045958,    0.0000021582674063,    0.0001440592899448,
		-0.0000000428581538,    0.0001622322743151,    -0.0000001042761430,    0.0001818708019650,
		-0.0000001607895767,    0.0002029979926012,    -0.0000001809999381,    0.0002256499142878,
		-0.0000001449098678,    0.0002498371451223,    -0.0000000796812276,    0.0002756082372421,
		-0.0000000506216023,    0.0003031217699056,    -0.0000001073899354,    0.0003326199643021,
		-0.0000002129872611,    0.0003643020827049,    -0.0000002052287741,    0.0003980697354330,
		0.0000000278900410,    0.0004335176928815,    0.0000001614927666,    0.0004708128391678,
		-0.0000004172634266,    0.0005115933884211,    0.0000000176787297,    0.0005536546622183,
		-0.0000001849469051,    0.0005986339809044,    -0.0000001371644143,    0.0006460536769266,
		-0.0000001215104853,    0.0006961208478517,    -0.0000001442285457,    0.0007489641395459,
		-0.0000001957557257,    0.0008047153456476,    -0.0000002180297631,    0.0008634755835951,
		-0.0000001665246477,    0.0009253245232310,    -0.0000000903703109,    0.0009903630339033,
		-0.0000001238926573,    0.0010587584496290,    -0.0000002408342631,    0.0011306657168229,
		-0.0000002010973113,    0.0012061247054804,    -0.0000000771212112,    0.0012852483193891,
		-0.0000002892183437,    0.0013683155959127,    -0.0000001435241686,    0.0014553239574353,
		-0.0000002449177181,    0.0015465017506511,    -0.0000002655054130,    0.0016419839245538,
		-0.0000002654202343,    0.0017419529101161,    -0.0000002384163353,    0.0018465928236920,
		-0.0000002259836788,    0.0019560890778418,    -0.0000002528141455,    0.0020706198898401,
		-0.0000002727517867,    0.0021903836179706,    -0.0000002270017753,    0.0023156250725059,
		-0.0000001904785508,    0.0024465768743472,    -0.0000002349089495,    0.0025834376087981,
		-0.0000002376930646,    0.0027264756485976,    -0.0000001873921894,    0.0028760147192473,
		-0.0000002812905273,    0.0030322200492974,    -0.0000002276024032,    0.0031955621147827,
		-0.0000002502466326,    0.0033662963043093,    -0.0000002606048313,    0.0035448023583031,
		-0.0000002769235368,    0.0037314620501271,    -0.0000002758829953,    0.0039267185318265,
		-0.0000002555715887,    0.0041310754093512,    -0.0000002445359290,    0.0043450290868452,
		-0.0000002483329736,    0.0045690930680355,    -0.0000002293829791,    0.0048039148431990,
		-0.0000001996153404,    0.0050501892299286,    -0.0000002005116227,    0.0053085881592326,
		-0.0000001941043677,    0.0055799597895743,    -0.0000001700015219,    0.0058652866115900,
		-0.0000001878836658,    0.0061653954641234,    -0.0000001765279505,    0.0064815477950166,
		-0.0000001806470858,    0.0068149450004986,    -0.0000001828037111,    0.0071670211602779,
		-0.0000001890179551,    0.0075393491459137,    -0.0000001885961077,    0.0079337018826826,
		-0.0000001828152179,    0.0083521589906808,    -0.0000001766524286,    0.0087970647057614,
		-0.0000001703017550,    0.0092710485425240,    -0.0000001664322980,    0.0097772209309617,
		-0.0000001592767069,    0.0103191830259970,    -0.0000001418591055,    0.0109010210861351,
		-0.0000001292084045,    0.0115276201369778,    -0.0000001351147310,    0.0122048043128822,
		-0.0000001254663478,    0.0129392736083207,    -0.0000001221494056,    0.0137392432812105,
		-0.0000001147145879,    0.0146145135754718,    -0.0000001079311634,    0.0155770652104650,
		-0.0000001114351109,    0.0166416128366098,    -0.0000001018156858,    0.0178264088528772,
		-0.0000001001025044,    0.0191544835985757,    -0.0000001014470290,    0.0206552259780350,
		-0.0000000808453104,    0.0223666966440739,    -0.0000000758199709,    0.0243392053813132,
		-0.0000000955186754,    0.0266405939341388,    -0.0000000870880452,    0.0293644914826645,
		-0.0000000712238573,    0.0326440240251014,    -0.0000000900373882,    0.0366750432945206,
		-0.0000000835031541,    0.0417577850953297,    -0.0000000895148496,    0.0483773634580267,
		-0.0000000888573772,    0.0573717000755485,    -0.0000000764996608,    0.0703236305613659,
		-0.0000000788050472,    0.0906250084654276,    -0.0000000510262652,    0.1270947385235992,
		-0.0000000360010810,    0.2120689964166065,    -0.0000000371645557,    0.6365738967733989,
		0.0000000000000000,    -0.6365738967733989,    0.0000000371645557,    -0.2120689964166065,
		0.0000000360010810,    -0.1270947385235992,    0.0000000510262652,    -0.0906250084654276,
		0.0000000788050472,    -0.0703236305613659,    0.0000000764996608,    -0.0573717000755485,
		0.0000000888573772,    -0.0483773634580267,    0.0000000895148496,    -0.0417577850953297,
		0.0000000835031541,    -0.0366750432945206,    0.0000000900373882,    -0.0326440240251014,
		0.0000000712238573,    -0.0293644914826645,    0.0000000870880452,    -0.0266405939341388,
		0.0000000955186754,    -0.0243392053813132,    0.0000000758199709,    -0.0223666966440739,
		0.0000000808453104,    -0.0206552259780350,    0.0000001014470290,    -0.0191544835985757,
		0.0000001001025044,    -0.0178264088528772,    0.0000001018156858,    -0.0166416128366098,
		0.0000001114351109,    -0.0155770652104650,    0.0000001079311634,    -0.0146145135754718,
		0.0000001147145879,    -0.0137392432812105,    0.0000001221494056,    -0.0129392736083207,
		0.0000001254663478,    -0.0122048043128822,    0.0000001351147310,    -0.0115276201369778,
		0.0000001292084045,    -0.0109010210861351,    0.0000001418591055,    -0.0103191830259970,
		0.0000001592767069,    -0.0097772209309617,    0.0000001664322980,    -0.0092710485425240,
		0.0000001703017550,    -0.0087970647057614,    0.0000001766524286,    -0.0083521589906808,
		0.0000001828152179,    -0.0079337018826826,    0.0000001885961077,    -0.0075393491459137,
		0.0000001890179551,    -0.0071670211602779,    0.0000001828037111,    -0.0068149450004986,
		0.0000001806470858,    -0.0064815477950166,    0.0000001765279505,    -0.0061653954641234,
		0.0000001878836658,    -0.0058652866115900,    0.0000001700015219,    -0.0055799597895743,
		0.0000001941043677,    -0.0053085881592326,    0.0000002005116227,    -0.0050501892299286,
		0.0000001996153404,    -0.0048039148431990,    0.0000002293829791,    -0.0045690930680355,
		0.0000002483329736,    -0.0043450290868452,    0.0000002445359290,    -0.0041310754093512,
		0.0000002555715887,    -0.0039267185318265,    0.0000002758829953,    -0.0037314620501271,
		0.0000002769235368,    -0.0035448023583031,    0.0000002606048313,    -0.0033662963043093,
		0.0000002502466326,    -0.0031955621147827,    0.0000002276024032,    -0.0030322200492974,
		0.0000002812905273,    -0.0028760147192473,    0.0000001873921894,    -0.0027264756485976,
		0.0000002376930646,    -0.0025834376087981,    0.0000002349089495,    -0.0024465768743472,
		0.0000001904785508,    -0.0023156250725059,    0.0000002270017753,    -0.0021903836179706,
		0.0000002727517867,    -0.0020706198898401,    0.0000002528141455,    -0.0019560890778418,
		0.0000002259836788,    -0.0018465928236920,    0.0000002384163353,    -0.0017419529101161,
		0.0000002654202343,    -0.0016419839245538,    0.0000002655054130,    -0.0015465017506511,
		0.0000002449177181,    -0.0014553239574353,    0.0000001435241686,    -0.0013683155959127,
		0.0000002892183437,    -0.0012852483193891,    0.0000000771212112,    -0.0012061247054804,
		0.0000002010973113,    -0.0011306657168229,    0.0000002408342631,    -0.0010587584496290,
		0.0000001238926573,    -0.0009903630339033,    0.0000000903703109,    -0.0009253245232310,
		0.0000001665246477,    -0.0008634755835951,    0.0000002180297631,    -0.0008047153456476,
		0.0000001957557257,    -0.0007489641395459,    0.0000001442285457,    -0.0006961208478517,
		0.0000001215104853,    -0.0006460536769266,    0.0000001371644143,    -0.0005986339809044,
		0.0000001849469051,    -0.0005536546622183,    -0.0000000176787297,    -0.0005115933884211,
		0.0000004172634266,    -0.0004708128391678,    -0.0000001614927666,    -0.0004335176928815,
		-0.0000000278900410,    -0.0003980697354330,    0.0000002052287741,    -0.0003643020827049,
		0.0000002129872611,    -0.0003326199643021,    0.0000001073899354,    -0.0003031217699056,
		0.0000000506216023,    -0.0002756082372421,    0.0000000796812276,    -0.0002498371451223,
		0.0000001449098678,    -0.0002256499142878,    0.0000001809999381,    -0.0002029979926012,
		0.0000001607895767,    -0.0001818708019650,    0.0000001042761430,    -0.0001622322743151,
		0.0000000428581538,    -0.0001440592899448,    -0.0000021582674063,    -0.0005668243045958,
		0.0000027971672683
};


//// calculate worst case bit-growth
//static double firCoefAcc = 0;
//double firBitGrowth = 0;
//
//for (i=0;i<numCoef;i++) {
//
//	if (hilbertCoef[i] < 0) {
//		firCoefAcc+=(double)-hilbertCoef[i];
//	} else {
//		firCoefAcc+=(double)hilbertCoef[i];
//	}
//	//fprintf(stdout,"taps[%4d]=%10.7f\n", i, firCoef_fx[i].to_double());
//}
//
//firBitGrowth = ceil(log2(firCoefAcc));
//std::cout<<"fir coef accumulation = "<<firCoefAcc<<std::endl;
//fprintf(stdout,"worst case output bit-growth = %f\n",firBitGrowth);

// *-----------------------------------------------------------------------------------* //

#endif

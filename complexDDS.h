/*------------------------------------------------------------------------------------------------*/
/* ___::((complexDDS.h))::___

   ___::((ZoTech IP))::___
   ___::((eschei))___

	Purpose: DDS - Direct Digital Synthesis IP
	Device: All
	Revision History: Feb 08, 2017 - initial
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*

HLS - C++ for FPGA
Direct Digital Synthesis IP
Supports dynamic frequency control and phase offset
Taylor Series Noise Reduction determined by design parameters

*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#ifndef __DDS_H__
#define __DDS_H__

#include <ap_int.h>
#include "ssbHilbertMod2_types.h"
#include "SinCos.h"


template <int dataWidth=24, int phaseWidth=36, int lutAddrWidth=7>
class complexDDS {

	ddsPhase_t phase;
	ddsPhase_t step;
	ddsPhase_t offset;

	SinCos<dataWidth, phaseWidth, lutAddrWidth> sc;

public:
	complexDDS() {
		step=1; //Just a random number
		offset=0;

	}

	void reset() { phase=0; }

	void set_step(ddsPhase_t x)
	{
		step=x;
	}

	void set_offset(ddsPhase_t x)
	{
		offset=x;
	}

	cmplx_t advance() {
#pragma HLS RESOURCE variable=phase core=AddSub_DSP
#pragma HLS PIPELINE

		cmplx_t ddsOut;

		ap_uint<phaseWidth+1> phaseNormCoef=1;
		phaseNormCoef<<=phaseWidth;

		ddsOut=sc.sin_taylor(phase+offset);
		phase+=step;

		return ddsOut;
	}

};


#endif

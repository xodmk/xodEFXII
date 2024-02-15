/*------------------------------------------------------------------------------------------------*/
/* ___::((xodCrossFader.h))::___

   ___::((XODMK))::___
   ___::((eschei))___

	Purpose: Equal Power CrossFade (Cos)
	Device: All
	Revision History: Feb 08, 2017 - initial
	Revision History: May 12, 2018 - ssbMod update
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*

HLS - C++ for FPGA
Equal Power Crossfade (Cosine Look-Up-Table)


*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#ifndef __XODCROSSFADER_H__
#define __XODCROSSFADER_H__

#include <math.h>
#include <utility>

using namespace std;


//const int ddsLutAddrWidth=7;						    // sincos LUT addr width
//// lut_table_size=1<<lut_addr_width;		        // sincos LUT table depth - defined in class
//
//
//// *--------------------------------------------------------------* //
//
//template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
//class CrossFader {
//	T Lut[1<<lutAddrWidth];
//
//public:
//	CrossFader();
//	~CrossFader() {}
//	T get(ap_uint<lutAddrWidth+1> x);
//	pair<T, T> xfade_lut(ap_uint<lutAddrWidth+2> x);
//};
//
//
//template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
//CrossFader<T, dataWidth, phaseWidth, lutAddrWidth>::CrossFader() {
////#pragma HLS RESOURCE variable=Lut latency=2
//
//	int lutTableSize = 1<<lutAddrWidth;
//
//	for (int i = 0; i < lutTableSize; i++) {
//		Lut[i] = cos(M_PI / (2 * lutTableSize) * i);
//		//std::cout<<"Lut["<<i<<"]="<<Lut[i]<<std::endl;
//	}
//}
//
//
//template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
//T CrossFader<T, dataWidth, phaseWidth, lutAddrWidth>::get(ap_uint<lutAddrWidth+1> x) {
//	if(x & (1<<lutAddrWidth))
//		return 1;
//	else
//		return Lut[x & ((1<<lutAddrWidth)-1)];
//}
//
//
//template <class T, int dataWidth, int phaseWidth, int lutAddrWidth>
//T CrossFader<T, dataWidth, phaseWidth, lutAddrWidth>::cos_lut(ap_uint<lutAddrWidth+2> x) {
//
//	T r;
//	ap_uint<lutAddrWidth> mx;
//
//	mx = x & ((1 << lutAddrWidth) - 1);
//
//	int lutTableSize = 1<<lutAddrWidth;
//
//	r = get(mx);
//
//	return r;
//}


// *--------------------------------------------------------------* //

#endif


//# /////////////////////////////////////////////////////////////////////////////
//# #############################################################################
//# begin : *---::XODMK Programmable Cross-Fader 1::---*
//# #############################################################################
//# \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//
//
//class xodXfade:
//    ''' Programmable Cross-Fade version 1
//
//    '''
//
//    def __init__(self, fs=48000):
//
//        # *---set primary parameters from inputs---*
//
//        self.fs = fs
//        self.phaseAcc = 0
//        self.tableDepth = 4096;
//
//        self.tb = self.tablegen(self.tableDepth);
//
//        #print('\nAn odmkSigGen1 object has been instanced with:')
//        #print('sigLength = '+str(self.sigLength)+'; fs = '+str(fs))
//
//
//
//    # // *-----------------------------------------------------------------* //
//
//
//    def tablegen(self, depth):
//        ''' cos 1/4 cycle '''
//
//        # tl = np.linspace(0, 1/(depth-1), 1)    # ?? check this linspace
//        tl = np.linspace(0.0, 1.0, depth)
//        table1 = np.array([])
//        # store 1 cycle
//        for q in range(len(tl)):
//            # 2 multiple necessary because function range changes from
//            # [0:2] to [0:1] for normalization
//            table1 = np.append(table1, np.cos(2 * np.pi/4 * tl[q]) )
//
//        return table1
//
//
//
//    # // *-----------------------------------------------------------------* //
//
//    # crossfade wavetable function
//    # ensures equal power crossfade by overlapped cos fade
//
//    def xtable(self, addr):
//
//        ''' *--------------------------------------------------------*
//        # odmkXfade: xtable 1/4 cos look-up table
//        # linear interpolation
//        #  p(x) = f(x0) + (f(x1)-f(x0))/(x1-x0)*(x-x0) // assume x1-x0 = 1
//        #  p(x) = f(x0) + (f(x1)-f(x0))*(x-x0)
//        # *--------------------------------------------------------* // '''
//
//
//        #tb = self.tablegen(self.tableDepth);
//
//        # assume address input is a value between 0 and 1 - scale to log2(depth) address bits
//        # scale addr to match table depth -> +1 for scilab 1 - n addressing
//        xaddr = addr * (2**(np.log2(self.tableDepth)) -2) + 1
//        # quantize for use as table address
//        qaddr = int(np.floor(xaddr))
//        # linear interpolate output (iout)
//        yHigh = self.tb[(qaddr+1)%self.tableDepth]
//        yLow = self.tb[qaddr]
//        iout = yLow + (yHigh - yLow)*(xaddr - qaddr)
//
//        return iout
//
//
//    # // *-----------------------------------------------------------------* //
//
//    # crossfade wavetable function
//    # ensures equal power crossfade by overlapped cos fade
//
//    def xfade(self, x, n):
//
//        ''' *--------------------------------------------------------*
//        # odmkWTOsc1: single channel wavetable oscillator
//        # *--------------------------------------------------------* // '''
//
//        # initialize internals
//        x1=0
//        y1=0
//        n1=0
//        x2=0
//        y2=0
//
//        # initialize outputs
//        fx2=0
//        fy2=0
//        #cpower2=0
//
//        x1 = 2*x - 1
//        y1 = 1 - 2*x
//        n1 = 2*n - 1
//
//        # multiply-accumulator used to implement exponential
//        # when x is normalized to [0:1], macc output (intx2,inty2) has range [0:2]
//        for i in range(n1):
//            if n1 == 1:
//                x2 = x1+1
//                y2 = y1+1
//            elif (n1 > 1 and i==1):
//                x2 = x1
//                y2 = y1
//            elif (n1 > 1 and i==n1):
//                x2 = x2*x1+1
//                y2 = y2*y1+1
//            else:
//                x2 = x2*x1
//                y2 = y2*y1
//
//
//        # temp, replace with look-up table
//        # fx2_tap1=cos(%pi/4*(intx2));
//        # fy2_tap1=cos(%pi/4*(inty2));
//
//
//        fx2 = self.xtable(x2/2)    # scale inputs from range [0:2] -> [0:1]
//        fy2 = self.xtable(y2/2)
//
//
//        # constant power equation:
//        #cpower2 = fx2**2 + fy2**2
//        #return fx2, fy2, cpower2
//
//        return fx2, fy2
//
//
//
//    def __call__(self, chA, chB, xlength, n):
//
//        ts = np.linspace(0, 1, xlength)
//
//        for s in range(xlength):
//            [xfA, xfB] = self.xfade(ts[s], n)
//
//            xf = chA*xfA + chB*xfB
//
//            yield xf

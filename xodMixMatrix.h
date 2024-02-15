/*------------------------------------------------------------------------------------------------*/
/* ___::((xodMixMatrix.h))::___

   ___::((JIROBATA Programming Industries))::___
   ___::((XODMK:2022))::___
   ___::((created by eschei))___

	Purpose: header .h for Digital Delay
	Device: All
	Revision History: July 23, 2022 - initial
*/

/*------------------------------------------------------------------------------------------------*/

/*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*
*---%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%---*/

/*------------------------------------------------------------------------------------------------*/

#include <cmath>

#include "xodEFXII_types.h"


// *--------------------------------------------------------------------------------------* //

// Use: 'Householder<data_t, numChan>::house(arrayIn, arrayOut)' - size must be ≥ 1
// ex:   Householder<dataExt_t, static_cast<int>(MAXCHANNELS)>::house(houseIn, houseOut);
template<typename T, int size>
class Householder {

public:
	static void house(T *arr, T *res) {
#pragma HLS PIPELINE
		T sum = 0;

		houseAcc_loop: for (int i = 0; i < size; ++i) {
#pragma HLS UNROLL
			sum += arr[i];
		}
		// const float houseScaling = -2.0 / MAXCHANNELS; (xodReverbIITypes.h)
		sum *= static_cast<T>(houseScaling);

		houseRes_loop: for (int i = 0; i < size; ++i) {
#pragma HLS UNROLL
			res[i] = arr[i] + sum;
		}
	};
};



//// Use like `Householder<double, 8>::inPlace(data)` - size must be ≥ 1
//template<typename Sample, int size>
//class Householder {
//	static constexpr Sample multiplier{-2.0/size};
//public:
//	static void inPlace(Sample *arr) {
//		double sum = 0;
//		for (int i = 0; i < size; ++i) {
//			sum += arr[i];
//		}
//
//		sum *= multiplier;
//
//		for (int i = 0; i < size; ++i) {
//			arr[i] += sum;
//		}
//	};
//};


// *--------------------------------------------------------------------------------------* //
// TEMP...

////Tail recursive call
//template<data_t N> struct fibon_s {
//	template<typename T>
//	static T fibon_f(T a, T b) {
//		return fibon_s<N-1>::fibon_f(b, (a+b));
//	}
//};
//
//// Termination condition
//template<> struct fibon_s<1> {
//	template<typename T>
//	static T fibon_f(T a, T b) {
//		return b;
//	}
//};
//
//void cpp_template(data_t a, data_t b, data_t &dout){
//	dout = fibon_s<FIB_N>::fibon_f(a,b);
//}

// *--------------------------------------------------------------------------------------* //


//Tail recursive call
template<int size> struct hadamardRecurs {
	template<typename T>
	static void recursiveUnscaled(T * data) {
		assert(size == 2 || size == 4 || size == 8 || size == 16);
		int hSize = size / 2;

//#ifndef __SYNTHESIS__
//		//TEMP PROBE it
//		static int recursCnt = 0;
//		recursCnt++;
//		std::cout << "xodMixMatrix::recursiveUnscaled recursCnt = " << recursCnt << std::endl;
//		for (int i = 0; i < size; ++i) {
//			std::cout << "xodMixMatrix::recursiveUnscaled din[" << i << "]: size = " << size << ",  hSize = " << hSize
//					  << ",  input data = " << data[i] << ",  input data + hsize = " << *(data + hSize) << std::endl;
//		}
//#endif

		// Two (unscaled) Hadamards of half the size
		hadamardRecurs<size / 2>::recursiveUnscaled(data);
		hadamardRecurs<size / 2>::recursiveUnscaled(data + hSize);

		// Combine the two halves using sum/difference
		for (int i = 0; i < hSize; ++i) {
#pragma HLS UNROLL
			T a = data[i];
			T b = data[i + hSize];
			data[i] = static_cast<T>(a + b);
			data[i + hSize] = static_cast<T>(a - b);

//#ifndef __SYNTHESIS__
//			//TEMP PROBE it
//			std::cout << "xodMixMatrix::recursiveUnscaled sum/diff[" << i << "]: hSize = " << hSize
//					  << ",  a = " << a << ",  b = " << b
//					  << ",  data = " << data[i]
//					  << ",  data + hsize = " << data[i + hSize]
//					  << std::endl;
//#endif

		}

	}
};


// Termination condition
template<> struct hadamardRecurs<1> {
	template<typename T>
	static inline void recursiveUnscaled(T * data) {
//#ifndef __SYNTHESIS__
//		std::cout << "Hadamard - Termination condition..." << std::endl;
//#endif
		return;
	}
};


// *--------------------------------------------------------------------------------------* //


// Use: - 'Hadamard<double, 8>::hadrecurs(data)' (size must be a power of 2)
template<typename T, int size>
class Hadamard {


public:
	static inline void hadrecurs(T * dataIn, T * dataOut) {
	//inline void hadrecurs(T * dataIn, T * dataOut) {

		T hadamardIn[size];

		hadDin_loop: for (int i = 0; i < size; ++i) {
#pragma HLS UNROLL
			hadamardIn[i] = dataIn[i];
		}

		hadamardRecurs<size>::recursiveUnscaled(hadamardIn);

		//const float hadamardScaling = std::sqrt(1.0 / MAXCHANNELS); (xodReverbIITypes.h)
		dataExt_t scalingFactor = static_cast<dataExt_t>(hadamardScaling);

		for (int c = 0; c < size; ++c) {
#pragma HLS UNROLL
			dataOut[c] = hadamardIn[c] * scalingFactor;
		}


//#ifndef __SYNTHESIS__
//		//TEMP PROBE
//		for (int i = 0; i < size; ++i) {
//			std::cout << "xodMixMatrix hadrecurs: scalingFactor = " << scalingFactor
//					  << ",  dataOut[" << i << "] = " << dataOut[i] << std::endl;
//		}
//#endif


//#ifndef __SYNTHESIS__
//		//TEMP PROBE it
//		static int hadStepCnt = 0;
//		static int hadSampleCnt = 0;
//		if (hadStepCnt % 3 == 0) hadSampleCnt++;
//		hadStepCnt++;
//		if (hadSampleCnt >= PROBE_LOW && hadSampleCnt <= PROBE_HIGH) {
//			for (int i = 0; i < size; ++i) {
//				std::cout << "xodMixMatrix - hadrecurs: sample[" << hadSampleCnt
//						  << "],	step[" << hadStepCnt
//						  << "],	dataOut = " << dataOut[i] << std::endl;
//			}
//		}
//#endif


	}
};



//// Use like `Hadamard<double, 8>::inPlace(data)` - size must be a power of 2
//template<typename Sample, int size>
//class Hadamard {
//public:
//	static inline void recursiveUnscaled(Sample * data) {
//		if (size <= 1) return;
//		constexpr int hSize = size/2;
//
//		// Two (unscaled) Hadamards of half the size
//		Hadamard<Sample, hSize>::recursiveUnscaled(data);
//		Hadamard<Sample, hSize>::recursiveUnscaled(data + hSize);
//
//		// Combine the two halves using sum/difference
//		for (int i = 0; i < hSize; ++i) {
//			double a = data[i];
//			double b = data[i + hSize];
//			data[i] = (a + b);
//			data[i + hSize] = (a - b);
//		}
//	}
//
//	static inline void inPlace(Sample * data) {
//		recursiveUnscaled(data);
//
//		Sample scalingFactor = std::sqrt(1.0/size);
//		for (int c = 0; c < size; ++c) {
//			data[c] *= scalingFactor;
//		}
//	}
//};

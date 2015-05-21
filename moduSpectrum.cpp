

#include "stdafx.h"

#include <io.h>
#include <stdio.h>
// for _O_RDONLY etc.
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <FLOAT.H>
#include <direct.h>

#include "parameters.h"

#include "procobj.h"
#include "fft\fft.h"

void DCT2D(const double ff[], int HH, int WW, double dct[], int HHO, int WWO);

#ifdef __USE_MODULATION_SPECTRUM

// ���� m_pBandSum[...] ���������㣬���ԣ��ڵ�Ԫ���ࣨ/��Ƶ�ֶΣ��׶Σ���Ԫ��ʱҪǰ�Ƹ����ݣ�����

#ifdef __USE_MEL_BAND_VALUE_AS_AM_SOURCE
#define NUM_BANDS_02	(NUM_DIMS_AM_SORCE_DATA-4)
#else
#define NUM_BANDS_02	(NUM_DIMS_AM_SORCE_DATA-0)
#endif

void CProcessingObj::MakeFeatureVectorMSP()
{
	double* pdbBuf = Malloc(double, NUM_BANDS_02*BH_FFT_WIN_WIDTH/2);	// �м仺��
	cpxv_t* tdom = Malloc(cpxv_t, BH_FFT_WIN_WIDTH*2);
	cpxv_t* ffdom = tdom+BH_FFT_WIN_WIDTH;

	int iBandMel, iFrame;
	for (iBandMel=0; iBandMel<NUM_BANDS_02; iBandMel++) {// ��ÿ�� Mel Ƶ��
		// һ��Ƶ����֡����
		for (iFrame=0; iFrame<m_nNumFrames; iFrame++) {
			// Hamming windowing
			tdom[iFrame].re = (0.54-0.46*cos(TWO_PI*iFrame/m_nNumFrames))*m_pBandSum[iFrame][iBandMel];
			tdom[iFrame].im = 0.0;
		}
		for (; iFrame<BH_FFT_WIN_WIDTH; iFrame++) {// zero padding
			tdom[iFrame].re = 0.0;
			tdom[iFrame].im = 0.0;
		}
		FFT(BH_FFT_WIN_WIDTH, tdom, ffdom);
		for (int ibin=0; ibin<BH_FFT_WIN_WIDTH/2; ibin++) {
			pdbBuf[iBandMel*(BH_FFT_WIN_WIDTH/2)+ibin] = sqrt(ffdom[ibin].im*ffdom[ibin].im+ffdom[ibin].re*ffdom[ibin].re);
		}
	}
	free(tdom);

	// Modulation Spectrum ������������ַ��ά����
//	AreaOfMoments(pdbBuf, NUM_BANDS_02, BH_FFT_WIN_WIDTH/2, m_pFeatureMSP, NUM_DIMS_MSP);
	DCT2D(pdbBuf, NUM_BANDS_02, BH_FFT_WIN_WIDTH/2, m_pFeatureMSP, 5, 5);

	free(pdbBuf);
}

#endif	// __USE_BEAT_HISTOGRAM

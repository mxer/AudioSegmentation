

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

void LPC(const short xxx[], int NNN, double aaa_out[], int ppp);
void STE(const short xxx[], int NNN, double& ste);
void DCT2D(const double ff[], int HH, int WW, double dct[], int HHO, int WWO);

void SpectralShape(const cpxv_t fdom[], double pSpectralShape[]);
void MakeFeatureVectorSpectralShape();
double ZeroCrossing(const short xxx[], int nNumSamples);

// ��һ�ּ��� MFCC �ķ����������զ��
int CalculateMFCC(const short *pSamps, int frameSize, double *pMfcc, int ceporder, int fftWinSize);

int CalcMfccFromMagnitudeSpectrum(const cpxv_t* fdom, double cepc_out[], int numCepstraOut);

void CProcessingObj::ProcSingleFrame(const short psamps[])
{
	// ��ǰ֡������Ҷ�任
	DoFFT(psamps, m_fdom);
	PrepareSrcAM();

// "m_nNumFrames" ��ǰ֡���±꣡����

/*
	// ��һ�ּ��� MFCC �ķ����������զ��
	CalculateMFCC(psamps, NumSamplesInFrameM, m_pdbMFCC+NumDimsMfccM*m_nNumFrames, NumDimsMfccM-1, NumBinsInFftWinM);
*/
//	CalcMfccFromMagnitudeSpectrum(m_fdom, m_pdbMFCC+NumDimsMfccM*m_nNumFrames, NumDimsMfccM);

	// ���㵱ǰ֡�� MFCC ����
	CalcMFCC(m_fdom, 
		// MFCC �������������ַ
		m_pdbMFCC+NumDimsMfccM*m_nNumFrames, NumDimsMfccM,	// NULL, 0,
		// ���� Mel band ��ֵ���� AM Դ���ݻ�������һ֡
//		m_pBandSum[NumFramesAreaMomentsSrcM-1];
		// ���� Mel band ��ֵ���� AM Դ���ݻ��浱ǰ֡
#ifdef __USE_MEL_BAND_VALUE_AS_AM_SOURCE
		m_pBandSum[m_nNumFrames]);	// �� Mel Ƶ�����ֺʹ�ֵ���㷨
#else
		NULL);	// ����Ҫ�Լ�����Ƶ��
	// �Լ��Ĵ�ֵ���㷽��
	void BandSum(const cpxv_t fdom[], double bandSums[]);
	BandSum(m_fdom, m_pBandSum[m_nNumFrames]);	// �� 7 ���˶ȹ鼯
#endif	// __USE_MEL_BAND_VALUE_AS_AM_SOURCE

	if (m_nNumFrames+1 >= NumFramesAreaMomentsSrcM) {// AM Դ���ݻ����е�Դ���ݹ�����һ�� AM ���������ˣ�
/*
		AreaOfMoments(m_pBandSum[m_nNumFrames+1-NumFramesAreaMomentsSrcM], NumFramesAreaMomentsSrcM, NUM_DIMS_AM_SORCE_DATA, 
			// AM ������������ַ��ά����
			m_pdbAM+NumDimsAreaMomentsM*m_nNumFrames, NumDimsAreaMomentsM);*/
		AreaOfMoments(&m_pSrcAM[0][0], NumFramesAreaMomentsSrcM, WIDTH_SOURCE_DATA_AM,
			m_pdbAM+NumDimsAreaMomentsM*m_nNumFrames, NumDimsAreaMomentsM);
/*		// ���� MFCC �� AM��Ч��û���ƣ�
		AreaOfMoments(m_pdbMFCC+(m_nNumFrames+1-NumFramesAreaMomentsSrcM)*NumDimsMfccM,
			NumFramesAreaMomentsSrcM, NumDimsMfccM, 
			m_pdbAM+NumDimsAreaMomentsM*m_nNumFrames, NumDimsAreaMomentsM);*/
/*		DCT2D(m_pBandSum[m_nNumFrames+1-NumFramesAreaMomentsSrcM], NumFramesAreaMomentsSrcM, NUM_DIMS_AM_SORCE_DATA,  
			m_pdbAM+NumDimsAreaMomentsM*m_nNumFrames, 4, 5);*/
	}

	// ��� 2 ��������LPC �����������λ��
	LPC(psamps, NumSamplesInFrameM, m_pLPC+NUM_DIMS_LPC*m_nNumFrames, NUM_DIMS_LPC);

#ifdef __USE_SPECTRAL_SHAPE_FEATURES
	SpectralShape(m_fdom, m_pSpectralShape+NUM_DIMS_SPECTRAL_SHAPE*m_nNumFrames);
#endif	// #ifdef __USE_SPECTRAL_SHAPE_FEATURES

	// ��ʱ�����������ղ������ֵ���ʴ��ڲ��ص�Ч����Щ
	STE(psamps, NumSamplesInFrameSkipM, m_pDataSTE[m_nNumFrames]);	//NumSamplesInFrameM
#ifndef __TRAINING_PHASE
	if (m_pfSTE) {// ��ǰ֡�� STE ֵд����ʱ�ļ�������ȷ���ν�ʱ�ã�
		if (fwrite(m_pDataSTE+m_nNumFrames, sizeof(double), 1, m_pfSTE) != 1) {
		}
	}
#endif	// __TRAINING_PHASE

#ifdef __USE_ZERO_CROSSING
	// ÿ֡��һ��ֵ
	m_pZC[m_nNumFrames] = ZeroCrossing(psamps, NumSamplesInFrameSkipM);
#endif	// __USE_ZERO_CROSSING

#ifdef __USE_SPECTRAL_SHAPE_FEATURES
	// ����������������
//	CalcPre();
#endif
}

bool IsSilent(const double xxx[], int nNumFrames);

int CProcessingObj::MakeFeatureVector()
{
/*	// "m_pDataSTE[...]" ��ʱ��֡���ź�ֵ����ֵ֮ƽ��ֵ���У���Ӧһ����Ԫ unit��
	if (IsSilent(m_pDataSTE, m_nNumFrames)) return -1;	// �������*/
	if (m_nNumFrames < NumFramesAreaMomentsSrcM) {
		printf("m_nNumFrames < NumFramesAreaMomentsSrcM !\n");
		return -2;	// ���ܼ��� AM ��������
	}

	// ���յ���������
	memset(m_pvector, 0, sizeof(double)*m_nNumDims);

	MakeFeatureVectorMFCC();
	MakeFeatureVectorAM();
	MakeFeatureVectorLPC();

#ifdef __USE_SPECTRAL_SHAPE_FEATURES
	MakeFeatureVectorSpectralShape();
#endif	// #ifdef __USE_SPECTRAL_SHAPE_FEATURES

#ifdef __USE_STE_FEATURES
	MakeFeatureVectorSTE();
#endif	// __USE_STE_FEATURES
#ifdef __USE_BEAT_HISTOGRAM
	MakeFeatureVectorBH();
#endif
#ifdef __USE_MODULATION_SPECTRUM
	MakeFeatureVectorMSP();
#endif
#ifdef __USE_ZERO_CROSSING
	MakeFeatureVectorZC();
#endif	// __USE_ZERO_CROSSING
#ifdef __USE_SUB_BAND_ENERGY
	MakeFeatureVectorSBE();
#endif	// __USE_SUB_BAND_ENERGY

	return 0;	// OK
}

#ifdef __USE_ZERO_CROSSING

void CProcessingObj::MakeFeatureVectorZC()
{
	if (m_pFeatureZC == NULL) return;

	double dbtemp;
	for (int ii=0; ii<m_nNumFrames-1; ii++) {
		for (int mm=ii+1; mm<m_nNumFrames; mm++) {
			if (m_pZC[ii] <= m_pZC[mm]) continue;
			dbtemp = m_pZC[ii];
			m_pZC[ii] = m_pZC[mm];
			m_pZC[mm] = dbtemp;
		}
	}
	dbtemp = m_pZC[m_nNumFrames-3]+m_pZC[m_nNumFrames-2]+m_pZC[m_nNumFrames-1];
	m_pFeatureZC[0] = (m_pZC[0]+m_pZC[1]+m_pZC[2])/dbtemp;

	return;
}

#endif	// __USE_ZERO_CROSSING



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
#include <TCHAR.h>

#include "parameters.h"

#include "procobj.h"
#include "fft\fft.h"

CProcessingObj::CProcessingObj()
{
	m_nNumValidFiles = 0;
//
	m_pfWaveform = new CWFFile;
	if (m_pfWaveform == NULL) {
		printf("m_pfWaveform == NULL !\n");
	}
/*	m_nNumTmpVectors = NumTmpVectors();
	m_pNumFrames = (int*)malloc(sizeof(int)*m_nNumTmpVectors);
	memset(m_pNumFrames, 0, sizeof(int)*m_nNumTmpVectors);*/
//
	m_fdom = Malloc(cpxv_t, NumBinsInFftWinM);
	if (m_fdom == NULL) {
		printf("m_fdom == NULL !\n");
	}

// AM Դ���ݣ�����֡��ÿ֡ "NUM_DIMS_AM_SORCE_DATA" ��ֵ --- ÿ��Ƶ��һ��ֵ������
	m_pBandSum = Malloc(BandSumArray_t, NumFrameSkipsInUnitM);	// *NumFramesAreaMomentsSrcM
	if (m_pBandSum == NULL) {
		printf("m_pBandSum == NULL !\n");
	}
	m_pSrcAM = Malloc(MagSpectArray_t, NumFramesAreaMomentsSrcM);
	if (m_pSrcAM == NULL) {
		printf("m_pSrcAM == NULL !\n");
	}

// �����������У�һ����Ԫ�еĸ��������������У�
	m_pdbMFCC = Malloc(double, NumDimsMfccM*NumFrameSkipsInUnitM);
	m_pdbAM = Malloc(double, NumDimsAreaMomentsM*NumFrameSkipsInUnitM);
	m_pLPC = Malloc(double, NUM_DIMS_LPC*NumFrameSkipsInUnitM);
	m_pDataSTE = Malloc(double, NumFrameSkipsInUnitM);	// ��ʱ��֡���ź�ֵ����ֵ֮ƽ��ֵ���У���Ӧһ����Ԫ unit��
	if (m_pdbMFCC == NULL || m_pdbAM == NULL || m_pLPC == NULL || m_pDataSTE == NULL) {
		printf("m_pdbMFCC == NULL || ... || m_pDataSTE == NULL !\n");
	}
//
	m_pMeanMFCC = m_pStdMFCC = m_pMeanDeriMFCC = m_pStdDeriMFCC = NULL;
	m_pMeanAM = m_pStdAM = m_pMeanDeriAM = m_pStdDeriAM = NULL;
	m_pMeanLPC = m_pStdLPC = m_pMeanDeriLPC = m_pStdDeriLPC = NULL;

#ifdef __USE_STE_FEATURES
	m_pFeatureSTE = NULL;
#endif

#ifdef __USE_SPECTRAL_SHAPE_FEATURES
/*	m_ftPre = Malloc(double, NumBinsInFftWinM);
	if (m_ftPre == NULL) {
		printf("m_ftPre == NULL !\n");
	}*/
	m_pSpectralShape = Malloc(double, NUM_DIMS_SPECTRAL_SHAPE*NumFrameSkipsInUnitM);
	if (m_pSpectralShape == NULL) {
		printf("m_pSpectralShape == NULL !\n");
	}
	m_pMeanSpectralShape = m_pStdSpectralShape = NULL;
#endif	// #ifdef __USE_SPECTRAL_SHAPE_FEATURES

#ifdef __USE_BEAT_HISTOGRAM
	m_pFeatureBH = NULL;
#endif	// __USE_BEAT_HISTOGRAM

#ifdef __USE_ZERO_CROSSING
	m_pZC = Malloc(double, NumFrameSkipsInUnitM);
	if (m_pZC == NULL) {
		printf("m_pZC == NULL !\n");
	}
	m_pFeatureZC = NULL;
#endif	// __USE_ZERO_CROSSING
#ifdef __USE_SUB_BAND_ENERGY
	m_pFeatureSBE = NULL;
#endif	// __USE_SUB_BAND_ENERGY

	m_nNumDims = 0;
// ��������������
	m_nNumDims += NumDimsMfccM*2;
	m_nNumDims += NumDimsAreaMomentsM*2;
	m_nNumDims += NUM_DIMS_LPC*2;
#ifdef __USE_STE_FEATURES
	m_nNumDims += NUM_STE_FEATURES;
#endif
#ifdef __USE_SPECTRAL_SHAPE_FEATURES
	m_nNumDims += NUM_DIMS_SPECTRAL_SHAPE;	// ��ֵ��
	m_nNumDims += NUM_DIMS_SPECTRAL_SHAPE;	// ����
#endif
#ifdef __USE_BEAT_HISTOGRAM
	m_nNumDims += NUM_DIMS_BH;
#endif	// __USE_BEAT_HISTOGRAM
#ifdef __USE_MODULATION_SPECTRUM
	m_nNumDims += NUM_DIMS_MSP;
#endif	// __USE_MODULATION_SPECTRUM
#ifdef __USE_ZERO_CROSSING
	m_nNumDims += NUM_DIMS_ZC;
#endif
#ifdef __USE_SUB_BAND_ENERGY
	m_nNumDims += NUM_DIMS_SBE;
#endif
	m_pvector = Malloc(double, m_nNumDims);
	if (m_pvector == NULL) {
		printf("m_pvector == NULL !\n");
	}
	if (m_pvector) {
		double* ptmp = m_pvector;

		m_pMeanMFCC = ptmp; ptmp += NumDimsMfccM;
		m_pStdMFCC = ptmp; ptmp += NumDimsMfccM;
/*		m_pMeanDeriMFCC = ptmp; ptmp += NumDimsMfccM;
		m_pStdDeriMFCC = ptmp; ptmp += NumDimsMfccM;*/
//
		m_pMeanAM = ptmp; ptmp += NumDimsAreaMomentsM;
		m_pStdAM = ptmp; ptmp += NumDimsAreaMomentsM;
/*		m_pMeanDeriAM = ptmp; ptmp += NumDimsAreaMomentsM;
		m_pStdDeriAM = ptmp; ptmp += NumDimsAreaMomentsM;*/
//
		m_pMeanLPC = ptmp; ptmp += NUM_DIMS_LPC;
		m_pStdLPC = ptmp; ptmp += NUM_DIMS_LPC;
/*		m_pMeanDeriLPC = ptmp; ptmp += NUM_DIMS_LPC;
		m_pStdDeriLPC = ptmp; ptmp += NUM_DIMS_LPC;*/

#ifdef __USE_STE_FEATURES
		m_pFeatureSTE = ptmp; ptmp += NUM_STE_FEATURES;
#endif	// __USE_STE_FEATURES
#ifdef __USE_SPECTRAL_SHAPE_FEATURES
		m_pMeanSpectralShape = ptmp; ptmp += NUM_DIMS_SPECTRAL_SHAPE;	// ��ֵ
		m_pStdSpectralShape = ptmp; ptmp += NUM_DIMS_SPECTRAL_SHAPE;	// ����
#endif	// __USE_SPECTRAL_SHAPE_FEATURES
#ifdef __USE_BEAT_HISTOGRAM
		m_pFeatureBH = ptmp; ptmp += NUM_DIMS_BH;
#endif	// __USE_BEAT_HISTOGRAM
#ifdef __USE_MODULATION_SPECTRUM
		m_pFeatureMSP = ptmp; ptmp += NUM_DIMS_MSP;
#endif	// __USE_MODULATION_SPECTRUM
#ifdef __USE_ZERO_CROSSING
		m_pFeatureZC = ptmp; ptmp += NUM_DIMS_ZC;
#endif
#ifdef __USE_SUB_BAND_ENERGY
		m_pFeatureSBE = ptmp; ptmp += NUM_DIMS_SBE;
#endif
	}

#ifndef __TRAINING_PHASE
//	m_pClassNo = Malloc(unsigned char, MAX_NUM_UNITS);	// ��Ÿ�����Ԫ������
	m_pWavInterface = NULL;	// ����أ�

	m_pfSTE = NULL;	// ��Ŵ��ֶ���Ƶ��֡����ֵ���ļ�
#endif	// #ifndef __TRAINING_PHASE
}

CProcessingObj::~CProcessingObj()
{
	ReleaseMemory();
/*	if (m_pNumFrames)
		free(m_pNumFrames);*/

#ifndef __TRAINING_PHASE
//	if (m_pClassNo) free(m_pClassNo);
	if (m_pWavInterface) delete m_pWavInterface;
#endif	// #ifndef __TRAINING_PHASE
}

// �ͷż�����������Ҫ���ڴ�
void CProcessingObj::ReleaseMemory()
{
// 111
	// wave form �ļ�����
	if (m_pfWaveform) { delete m_pfWaveform; m_pfWaveform = 0; }
// 222
	// һ֡�� FFT ���
	if (m_fdom) { free(m_fdom); m_fdom = 0; }
// 333
	// ֡ Mel Bands ��������
	if (m_pBandSum) { free(m_pBandSum); m_pBandSum = 0; }
	if (m_pSrcAM) { free(m_pSrcAM); m_pSrcAM = NULL; }
// 444
	// ֡ MFCC ������������
	if (m_pdbMFCC) { free(m_pdbMFCC); m_pdbMFCC = 0; }
// 555
	// ֡ AM ������������
	if (m_pdbAM) { free(m_pdbAM); m_pdbAM = 0; }
// 666
	// ֡ LPC ������������
	if (m_pLPC) { free(m_pLPC); m_pLPC = 0; }

// 777
	// ֡����ֵ����
	if (m_pDataSTE) { free(m_pDataSTE); m_pDataSTE = 0; }
// 888
#ifdef __USE_SPECTRAL_SHAPE_FEATURES
//	if (m_ftPre) { free(m_ftPre); m_ftPre = 0; }
	// ֡��ɫ������������
	if (m_pSpectralShape) { free(m_pSpectralShape); m_pSpectralShape = 0; }
#endif
#ifdef __USE_ZERO_CROSSING
	if (m_pZC) { free(m_pZC); m_pZC = 0; }
#endif	// __USE_ZERO_CROSSING
// aaa

	// ������������
	if (m_pvector) { free(m_pvector); m_pvector = 0; }

}

// ��ǰ��Ԫ������󣬶�Ҫ��һ��������һ��Ԫ�����˺������ڴ������ڵ�Ԫ����ص����֡�
void CProcessingObj::MoveUnitOverlap()
{
	if (NumFrameSkipsInUnitSkipM >= NumFrameSkipsInUnitM) {
		m_nNumFrames = 0;
		return;
	}

// ��һ�������������У�������

	// MFCC �����������У�������
	memmove(m_pdbMFCC, m_pdbMFCC+NumDimsMfccM*NumFrameSkipsInUnitSkipM, 
				sizeof(double)*NumDimsMfccM*(NumFrameSkipsInUnitM-NumFrameSkipsInUnitSkipM));
	// AM �����������У�������
	memmove(m_pdbAM, m_pdbAM+NumDimsAreaMomentsM*NumFrameSkipsInUnitSkipM, 
				sizeof(double)*NumDimsAreaMomentsM*(NumFrameSkipsInUnitM-NumFrameSkipsInUnitSkipM));
	// LPC �����������У�������
	memmove(m_pLPC, m_pLPC+NUM_DIMS_LPC*NumFrameSkipsInUnitSkipM,
				sizeof(double)*NUM_DIMS_LPC*(NumFrameSkipsInUnitM-NumFrameSkipsInUnitSkipM));

#ifdef __USE_SPECTRAL_SHAPE_FEATURES
	memmove(m_pSpectralShape, m_pSpectralShape+NUM_DIMS_SPECTRAL_SHAPE*NumFrameSkipsInUnitSkipM,
				sizeof(double)*NUM_DIMS_SPECTRAL_SHAPE*(NumFrameSkipsInUnitM-NumFrameSkipsInUnitSkipM));
#endif	//#ifdef __USE_SPECTRAL_SHAPE_FEATURES
#ifdef __USE_ZERO_CROSSING
	// ÿ֡һ��ֵ��1 ά��
	memmove(m_pZC, m_pZC+NumFrameSkipsInUnitSkipM,
				sizeof(double)*(NumFrameSkipsInUnitM-NumFrameSkipsInUnitSkipM));
#endif	// __USE_ZERO_CROSSING

// �������м����ݣ�������

	// ÿ֡ 1 ά
	memmove(m_pDataSTE, m_pDataSTE+NumFrameSkipsInUnitSkipM,
			sizeof(double)*(NumFrameSkipsInUnitM-NumFrameSkipsInUnitSkipM));
	// ֡ Mel Bands ��������
	memmove(m_pBandSum[0], m_pBandSum[NumFrameSkipsInUnitSkipM],
			sizeof(double)*(NumFrameSkipsInUnitM-NumFrameSkipsInUnitSkipM)*NUM_DIMS_AM_SORCE_DATA);

// ������

	m_nNumFrames = NumFrameSkipsInUnitM-NumFrameSkipsInUnitSkipM;
}















/*
// ���ݵ�Ԫ����֡�����͵�Ԫ������֡������Сȷ����Ҫ������ʱ֡����
int CProcessingObj::NumTmpVectors()
{
	int nNumTmpVectors = 1;
	int nstart = 0;
	while (nstart+NumFrameSkipsInUnitSkipM < NumFrameSkipsInUnitM) {
		nNumTmpVectors++;
		nstart += NumFrameSkipsInUnitSkipM;
	}
	return nNumTmpVectors;
}

int CProcessingObj::NumTmpVectorsToCompute(int iframe)
{
	int nTmpVecotrs = 1;
	int nstart = 0;
	while (nstart+NumFrameSkipsInUnitSkipM <= iframe) {
		nTmpVecotrs++;
		nstart += NumFrameSkipsInUnitSkipM;
	}
	return nTmpVecotrs;
}
*/



#ifndef	__FFT_FEATURES_H__
#define __FFT_FEATURES_H__

#include "..\mydefs.h"

int CalcTempFeatures(const short *pshSamps, int nNumSamps, double *pdbout);
int CalcSpectralFeatures(const cpxv_t *pFT, int nNumBins, double *pdbout);

int GetNumDimsMFCC();
int NumDimsFrameSpectralFeatures();
int NumDimsOtherFeatures();

//////////////////////////////////////////////////////////////////////////////////////////////////////

class CCalcMFCC {
private:
	int m_samplingRate;	// �źŲ�����
	short m_nNumBins;	// ����Ҷ�׷�������

public:
	void SetParams(int srate, short numbins);
//	void CalcMFCC_FFT(const cpxv_t *pFSP, double *pmfcc, int nNumMfccDims);
	int CalcFrameSpectralFeatures(const cpxv_t *pFSP, double *pdbfeatures);

public:
	CCalcMFCC(void);
	virtual ~CCalcMFCC(void);
};

#endif	// __FFT_FEATURES_H__

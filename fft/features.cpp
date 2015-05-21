// fft_dll.cpp : ���� DLL Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <stdlib.h>
#include <math.h>

#include "features.h"
#include "fft.h"
#include "..\parameters.h"

#define NUM_DIMENSIONS_CHROMA		13
#define NUM_DIMS_OF_OTHER_FEATURES	8

int GetNumDimsMFCC() { return NumDimsMfccM; }
int NumDimsFrameSpectralFeatures() { return NumDimsMfccM+NUM_DIMENSIONS_CHROMA; }
int NumDimsOtherFeatures() { return NUM_DIMS_OF_OTHER_FEATURES; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �����Ǽ��� MFCC ���࣬������������


#define NUM_CHROMA_BINS					32	// ��������������㷨������

#define CCC_FACTOR						(1.0E+5)

// �����ѵ�����Ĺ��캯����
CCalcMFCC::CCalcMFCC()
{
	return;
}
CCalcMFCC::~CCalcMFCC()
{
	return;
}

void CCalcMFCC::SetParams(int srate, short numbins)
{
	// ���� MFCC ʱ������֪���źŲ���Ƶ���Լ�����ҶƵ�׷�������
	m_samplingRate = srate;
	m_nNumBins = numbins;
}

int CCalcMFCC::CalcFrameSpectralFeatures(const cpxv_t *pFSP, double *pdbfeatures)
{
	CalcMFCC(pFSP, pdbfeatures, NumDimsMfccM, NULL);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
	double *XX = new double[NUM_CHROMA_BINS+1];

	double ff;
	int mmm, kkk;
	int ggg;	// Ϊ��ʡʱ�䣡
	for (mmm=1; mmm<=NUM_CHROMA_BINS; mmm++) {
		XX[mmm] = 0.0;
		ggg = 0;
		for (kkk=(mmm-1)*2+1; kkk<NUM_CHROMA_BINS*2; ) {// ���ڲ���Ƶ�ʶ���֮һ��Ƶ�׷������迼��
			if (ggg == 0) {
				ff = (double)kkk/m_nNumBins*m_samplingRate;	// ������Ӧ��Ƶ��
				if (ff >= 16.352) {// C0
					XX[mmm] += sqrt(pFSP[kkk].re*pFSP[kkk].re+pFSP[kkk].im*pFSP[kkk].im);
					ggg = 1;
				}
			} else {
				XX[mmm] += sqrt(pFSP[kkk].re*pFSP[kkk].re+pFSP[kkk].im*pFSP[kkk].im);
			}
			kkk *= 2;
		}
	}
	int nnn = 2;
	mmm = 1;	// ��һ��Ԫ�أ��±�Ϊ 0���ڴ��˷���
	int sss = nnn;
	for (kkk=NUM_CHROMA_BINS*2; kkk<m_nNumBins/2; kkk++) {
		XX[mmm] += sqrt(pFSP[kkk].re*pFSP[kkk].re+pFSP[kkk].im*pFSP[kkk].im);
		sss--;
		if (sss == 0) {
			mmm++;
			sss = nnn;
		}
		if (mmm > NUM_CHROMA_BINS) {
			nnn *= 2;
			mmm = 1;
			sss = nnn;
		}
	}
	for (mmm=1; mmm<=NUM_CHROMA_BINS; mmm++) {
		XX[mmm] = log(1.0+CCC_FACTOR*XX[mmm]);
	}

// ��ɢ���ұ任
	for (kkk=1; kkk<=NUM_DIMENSIONS_CHROMA; kkk++) {
		pdbfeatures[NumDimsMfccM+kkk-1] = 0.0;
		for (mmm=1; mmm<=NUM_CHROMA_BINS; mmm++) {
			pdbfeatures[NumDimsMfccM+kkk-1] += XX[mmm]*cos((double)kkk*MY_PI/NUM_CHROMA_BINS*((double)mmm-0.5));
		}
	}

	//////////////////////////////////////////////////////////////////////////////

	delete []XX;

	// ������ȡ����������
	return NumDimsMfccM+NUM_DIMENSIONS_CHROMA;
}

/*
int LPC(const short xxx[], int nNumSamples, double LpcCoefficients[]);*/

// ����Ӧ����ȡ��������������ʹ��ȡ���ɹ�����
// 
int CalcTempFeatures(const short *pshSamps, int nNumSamps, double *pdbout)
{
	int iDim = 0;	// ��ȡ����������

	double dbRMS = 0.0;
	int nNumCrossings = 0;
	short smap_pre = 0;
	for (int ii=0; ii<nNumSamps; ii++) {
		dbRMS += (double)pshSamps[ii]*pshSamps[ii];
		// ������
		if (smap_pre == 0) {
			if (pshSamps[ii] != 0) nNumCrossings++;
		} else if (smap_pre > 0) {
			if (pshSamps[ii] < 0) nNumCrossings++;
		} else {// smap_pre < 0
			if (pshSamps[ii] > 0) nNumCrossings++;
		}
		smap_pre = pshSamps[ii];
	}
	if (nNumSamps > 0) {
		dbRMS /= nNumSamps;
	}
	pdbout[iDim] = sqrt(dbRMS);
	iDim++;
//	if (fwrite(&, sizeof(double), 1, pfout) != 1) { }	// ��ʱ����

	dbRMS = nNumCrossings;
	if (nNumSamps > 0) {
		dbRMS /= nNumSamps;
	}
	pdbout[iDim] = dbRMS;
	iDim++;
//	if (fwrite(&, sizeof(double), 1, pfout) != 1) { }	// ����

//
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
/*	// LPC coefficients
	iDim += LPC(pshSamps, nNumSamps, pdbout+iDim);*/

	return iDim;
}

//int CalcSpectralFeatures02(const cpxv_t *pFT, int nNumBins, double *pdbout);

// ����Ӧ����ȡ��������������ʹ��ȡ���ɹ�����
// 
int CalcSpectralFeatures(const cpxv_t *pFT, int nNumBins, double *pdbout)
{
	int iDim = 0;

	// (11111) ����λ��
	double dbsum = 0.0;
	double centroid = 0.0;
	for (int iBin=0; iBin<nNumBins/2; iBin++) {
		centroid += (iBin+1)*sqrt(pFT[iBin].re*pFT[iBin].re+pFT[iBin].im*pFT[iBin].im);
		dbsum += sqrt(pFT[iBin].re*pFT[iBin].re+pFT[iBin].im*pFT[iBin].im);
	}
	if (dbsum > 0.0) {
		centroid = centroid/dbsum-1.0;
	}
	pdbout[iDim] = centroid;
	iDim++;
//	if (fwrite(&, sizeof(double), 1, pfout) != 1) { }	// ����λ��

	// (22222)
	double dbsquaredsigma = 0.0;
	for (int iBin=0; iBin<nNumBins/2; iBin++) {
		dbsquaredsigma += (iBin-centroid)*(iBin-centroid)*sqrt(pFT[iBin].re*pFT[iBin].re+pFT[iBin].im*pFT[iBin].im);
	}
	if (dbsum > 0.0) {
		dbsquaredsigma /=dbsum;
	}
	pdbout[iDim] = dbsquaredsigma;
	iDim++;
//	if (fwrite(&, sizeof(double), 1, pfout) != 1) { }	// ��չ��

	// (33333)
	double mm = 0.0;	// m3
	for (int iBin=0; iBin<nNumBins/2; iBin++) {
		mm += (iBin-centroid)*(iBin-centroid)*(iBin-centroid)*
						sqrt(pFT[iBin].re*pFT[iBin].re+pFT[iBin].im*pFT[iBin].im);
	}
	if (dbsum > 0.0) {
		mm /=dbsum;
	}
	if (dbsquaredsigma > 0.0) {
		mm /= dbsquaredsigma*sqrt(dbsquaredsigma);	// sigma to the power of 3
	}
	pdbout[iDim] = mm;
	iDim++;
//	if (fwrite(&, sizeof(double), 1, pfout) != 1) { }	// m3

	// (44444)
	mm = 0.0;	// m4
	for (int iBin=0; iBin<nNumBins/2; iBin++) {
		mm += (iBin-centroid)*(iBin-centroid)*(iBin-centroid)*(iBin-centroid)*
						sqrt(pFT[iBin].re*pFT[iBin].re+pFT[iBin].im*pFT[iBin].im);
	}
	if (dbsum > 0.0) {
		mm /=dbsum;
	}
	if (dbsquaredsigma > 0.0) {
		mm /= dbsquaredsigma*dbsquaredsigma;	// sigma to the 4th power
	}
	pdbout[iDim] = mm;
	iDim++;
//	if (fwrite(&, sizeof(double), 1, pfout) != 1) { }	// m4

	// (55555) spectral decrease
	dbsum = 0.0;
	mm = 0.0;
	centroid = sqrt(pFT[1].re*pFT[1].re+pFT[1].im*pFT[1].im);	// amplitude of the 2nd bin
	for (int iBin=2; iBin<nNumBins/2; iBin++) {// from the 3rd bin, ...
		dbsum += sqrt(pFT[iBin].re*pFT[iBin].re+pFT[iBin].im*pFT[iBin].im);
		mm += (sqrt(pFT[iBin].re*pFT[iBin].re+pFT[iBin].im*pFT[iBin].im)-centroid)/(iBin-1);
	}
	if (dbsum > 0.0) {
		mm /= dbsum;
	}
	pdbout[iDim] = mm;
	iDim++;
//	if (fwrite(&, sizeof(double), 1, pfout) != 1) { }	// 

	// (66666) spectral roll-off
	dbsum = 0.0;
	for (int iBin=0; iBin<nNumBins/2; iBin++) {// һ��
		// sum of squared amplitudes of half the bins
		dbsum += pFT[iBin].re*pFT[iBin].re+pFT[iBin].im*pFT[iBin].im;
	}
	mm = 0.0;
	for (int iBin=0; iBin<nNumBins/2; iBin++) {
		mm += pFT[iBin].re*pFT[iBin].re+pFT[iBin].im*pFT[iBin].im;	// summing, 
		if (mm > 0.95*dbsum) {// , until the sum exceeds 95% of the total
			mm = iBin;
			break;
		}
	}
	pdbout[iDim] = mm;
	iDim++;
//	if (fwrite(&, sizeof(double), 1, pfout) != 1) { }	// 

//	iDim += CalcSpectralFeatures02(pFT, nNumBins, pdbout+iDim);

	return iDim;
}

/*
void InsertElement(const double *pdbbuf, short iBin, short *pIdxPeak, short& nNumElements);
void SortList(short *pIdxPeak, short nNumElements);

//#define sin_quarter_pi	0.78539816339744830961566084581988
#define NUM_ELEMENTS_PEAK	20

// ����Ӧ����ȡ��������������ʹ��ȡ���ɹ�����
// 
int CalcSpectralFeatures02(const cpxv_t *pFT, int nNumBins, double *pdbout)
{
	double *pdbbuf = new double[nNumBins/2];
	if (pdbbuf == NULL) {
		return -1;
	}

	short iDim = 0;

// ���ֵ�ף�������
	for (int ii=0; ii<nNumBins/2; ii++) {
		pdbbuf[ii] = sqrt(pFT[ii].re*pFT[ii].re+pFT[ii].im*pFT[ii].im);
	}

// �Է�ֵ�׽����˲���Ϊ���������еĸ�Ƶ�ɷ֣�������
	double dbtmp01, dbtmp02;
	dbtmp01 = pdbbuf[0];
//	pdbbuf[0] = (pdbbuf[0]+pdbbuf[1]*sin_quarter_pi)/(1.0+sin_quarter_pi);
	pdbbuf[0] = (pdbbuf[0]+pdbbuf[1])/2.0;
	for (int ii=1; ii<nNumBins/2-1; ii++) {
		dbtmp02 = pdbbuf[ii];
//		pdbbuf[ii] = (dbtmp01*sin_quarter_pi+pdbbuf[ii]+pdbbuf[ii+1]*sin_quarter_pi)/(sin_quarter_pi+1.0+sin_quarter_pi);
		pdbbuf[ii] = (dbtmp01+pdbbuf[ii]+pdbbuf[ii+1])/3.0;
		dbtmp01 = dbtmp02;
	}
//	pdbbuf[nNumBins/2-1] = (dbtmp01*sin_quarter_pi+pdbbuf[nNumBins/2-1])/(sin_quarter_pi+1.0);
	pdbbuf[nNumBins/2-1] = (dbtmp01+pdbbuf[nNumBins/2-1])/2.0;

	//////////////////////////////////////////////////////////////////////////////////////////

	short nNumElementsPeak = 0;
	short *pIdxPeak = new short[NUM_ELEMENTS_PEAK];
	memset(pIdxPeak, 0, sizeof(short)*NUM_ELEMENTS_PEAK);

	dbtmp01 = 0.0;
	int nNumPeaks = 0;
	int iBin = -1;
	for (int ii=1; ii<nNumBins/2-1; ii++) {
		if (pdbbuf[ii-1] <= pdbbuf[ii] && pdbbuf[ii] > pdbbuf[ii+1]) {// ������ֵ��������
			InsertElement(pdbbuf, ii, pIdxPeak, nNumElementsPeak);	// ��㰴��ֵ�Ӵ�С����

			dbtmp01 += pdbbuf[ii];
			nNumPeaks++;
			if (iBin == -1) {
				pdbout[iDim] = ii;	// Ƶ����͵ģ�����ߵģ���ֵ��Ӧ��Ƶ��
				iDim++;
			}
			iBin = ii;
		}
	}
	SortList(pIdxPeak, nNumElementsPeak);	// ��㣨DFT bin number����Ƶ�ʴӴ�С����

	if (nNumPeaks > 1) {
		dbtmp01 /= nNumPeaks;
	}
	pdbout[iDim] = nNumPeaks;	// ��ֵ����
	iDim++;
	pdbout[iDim] = iBin;	// Ƶ����ߵģ����ұߵģ���ֵ��Ӧ��Ƶ��
	iDim++;

	dbtmp02 = 0.0;
	for (int ii=1; ii<nNumBins/2-1; ii++) {
		if (pdbbuf[ii-1] <= pdbbuf[ii] && pdbbuf[ii] > pdbbuf[ii+1]) {// ������ֵ��������
			dbtmp02 += (dbtmp01-pdbbuf[ii])*(dbtmp01-pdbbuf[ii]);
		}
	}
	if (nNumPeaks > 2) {
		dbtmp02 = sqrt(dbtmp02/(nNumPeaks-1));
	}
	pdbout[iDim] = log10(1.0E-10+dbtmp01);	// ��ֵ��ֵ
	iDim++;
	pdbout[iDim] = log10(1.0E-10+dbtmp02);	// ��ֵ����
	iDim++;

	delete []pIdxPeak;
	delete []pdbbuf;

	return 5;
}

// "nNumElements" number of elements currently in the array which hold the DFT bin numbers of peaks
// bin number ����Ӧ�ķ�ֵ�Ӵ�С���򣬡�����
// 
void InsertElement(const double *pdbbuf, short iBin, short BinIdxPeakAry[], short& nNumElements)
{
	short ii, kk;
	for (ii=0; ii<nNumElements; ii++) {
		if (pdbbuf[iBin] > pdbbuf[BinIdxPeakAry[ii]]) {
			break;
		}
	}
	kk = nNumElements-1;
	if (nNumElements == NUM_ELEMENTS_PEAK) {// ������������
		kk--;
	}
	for (; kk>=ii; kk--) {
		BinIdxPeakAry[kk+1] = BinIdxPeakAry[kk];
	}
	if (ii < NUM_ELEMENTS_PEAK) {
		BinIdxPeakAry[ii] = iBin;
	}
	if (nNumElements < NUM_ELEMENTS_PEAK) {
		nNumElements++;
	}
}

// ������Ԫ�أ���ֵ��Ӧ�� DFT bin number����С�������򣬡�����
// 
void SortList(short IdxBinPeakAry[], short nNumElements)
{
	short ii, kk, tmp;
	for (ii=0; ii<nNumElements; ii++) {
		for (kk=ii+1; kk<nNumElements; kk++) {
			if (IdxBinPeakAry[ii] < IdxBinPeakAry[kk]) {
				continue;
			}
			tmp = IdxBinPeakAry[ii];
			IdxBinPeakAry[ii] = IdxBinPeakAry[kk];
			IdxBinPeakAry[kk] = tmp;
		}
	}
}

void InsertElement(double SpectrumAry[], short nNumBins, const short BinIdxPeakAry[], short nNumElements, double OutAry[])
{
	double dbsum;
	short ibin, size, dd;
	for (short ii=0; ii<nNumElements; ii++) {
		dbsum = 0.0;
		ibin = size = BinIdxPeakAry[ii];
		while (ibin < nNumBins/2) {
			dbsum += SpectrumAry[ibin];
			SpectrumAry[ibin] = 0.0;
			ibin += size;
		}
		dd = 2;
		while ((ibin = size/dd) > 0) {
			dbsum += SpectrumAry[ibin];
			SpectrumAry[ibin] = 0.0;
			dd++;
		}

	}
}

*/


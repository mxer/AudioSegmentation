

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

#include "ErrorNumber.h"

#include "mydefs.h"
#include "procobj.h"
#include "fft\fft.h"

#ifndef __TRAINING_PHASE

extern FILE* g_pfwInfo;

#include <algorithm>
// ZONE_INFO_Ts will be sorted by ...
bool operator<(const ZONE_INFO_T& xx, const ZONE_INFO_T& yy)
{
	return xx.iRR-xx.iLL > yy.iRR-yy.iLL;
}

void DoSegmentation(const ZONE_VECTOR& zoneVector, int nNumFramesTotal,	std::vector<int>& segEndVector);

#define NUM_FRAME_SKIPS_BUFFER_CAN_HOLD	1024	// 1024 ֡

#define CCC_000		2.0

// ֡������
#define CCC_009		15

// 1.0 ��
#define CCC_010		5.0		// Լ�� 6.0 ��

// ����Ƶ�ֶΣ����������ε�ĩβ֡�±꣬���������ʣ��Ĳ���һ��֡���Ĳ�������
int CProcessingObj::SegmentAudio02(int fhr, std::vector<int>& segEndVector)
{
	// ���ʣ��Ĳ���һ��֡���Ĳ�������
	int nNumSamplesLeft = SegmentAudioHelper02(fhr);
	if (nNumSamplesLeft < 0) {
		return -1;
	}
//
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_STE);
	FILE *pfrSTE = fopen(m_szfn, "rb");	// ���ļ���Ϊ�˶����ݣ�������

	// ע�⣺�����֡����������֡�ĸ�����
	int nNumFramesTotal;
	if (fread(&nNumFramesTotal, sizeof(int), 1, pfrSTE) != 1) {
	}

	ZONE_VECTOR zoneVector;
	ZONE_INFO_T zoneInfo;

	size_t nBufSizeInDBs = sizeof(short)*NumSamplesInFrameSkipM*NUM_FRAME_SKIPS_BUFFER_CAN_HOLD/sizeof(double);
	double *pdbSTE = Malloc(double, nBufSizeInDBs);
	int iFrm_G = 0;	// ֡��ȫ�֣��±�
	int iLL = -1, iRR = -2;	// ��ѡ�з����������ҽ�֡�±�
	size_t nSTEsRead;
	while (1) {
		nSTEsRead = fread(pdbSTE, sizeof(double), nBufSizeInDBs, pfrSTE);
		if (nSTEsRead == 0) break;

		for (int ii = 0; ii<nSTEsRead; ii++, iFrm_G++) {// ע�� "iFrm_G++" ��
			if (pdbSTE[ii] <= CCC_000*MINIMUM_MEAN_ABS_SIGNAL_VALUE) {// ���ܵ��зֵ㣬������
				if (iLL < 0) {// ��ѡ�з�����磬һ��ȷ�������ɸ��ģ�
					iLL = iFrm_G; 
				}
				iRR = iFrm_G;	// �����ҽ�
				continue;
			}			
//
			if ((iRR-iLL+1) >= CCC_009) {// ��ѡ�з����㹻��
				assert(iRR >= 0);
				zoneInfo.iLL = iLL;
				zoneInfo.iRR = iRR;
				zoneVector.push_back(zoneInfo);
			}
			if (iLL >= 0) {
				iLL = -1; iRR = -2;
			}
		}
	}
	assert(iFrm_G == nNumFramesTotal);

// �ɼ�����Ƶ�ײ��ĺ�ѡ�з����������ˣ�����Ƶβ���ĺ�ѡ�з����������ˡ�

	// �����к�ѡ�з��������ȴӴ�С�����Ա����˳����֮
    sort(zoneVector.begin(), zoneVector.end());

//
////////////////////////////////////////////////////////////////////////////////////////////
//
	// ����Ƶ�ִ�Σ����������ε�ĩβ֡�±�
	DoSegmentation(zoneVector, nNumFramesTotal, segEndVector);

	// ����ΰ���ĩβ֡�±�֮��Ȼ˳������
    sort(segEndVector.begin(), segEndVector.end());
//
	free(pdbSTE);
	// �رղ�ɾ����ʱ�ļ���������
	fclose(pfrSTE);
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_STE);
	remove(m_szfn);
//
	if (g_pfwInfo) {
		fprintf(g_pfwInfo, "\n�����Ϣ���� %d ����Σ�������\n", segEndVector.size());
		int& nNumSampsTotal = iLL;
		nNumSampsTotal = segEndVector.back()*NumSamplesInFrameSkipM+NumSamplesInFrameM;	// ��Ƶ��֡��
		nNumSampsTotal += nNumSamplesLeft;

		int& iSample = iFrm_G;
		int iPreEnd = -1;	// �����±�
		for (std::vector<int>::size_type iSeg = 0; iSeg < segEndVector.size(); iSeg++) {
			ldiv_t lDivR;

			// ��ν���λ�ã������±��Ӧ��ʱ�䣩����ĩ��Σ����Բ����Ĳ���һ֡���Ĳ�����
			iSample = segEndVector[iSeg]*NumSamplesInFrameSkipM+NumSamplesInFrameM;
			if (iSeg == segEndVector.size()-1) {
				iSample += nNumSamplesLeft;	// ���һ����μ��ϲ�ֵ
			} else {
				iSample -= NumSamplesInFrameM-NumSamplesInFrameSkipM;
			}
			lDivR.rem = 1000.0*(iSample-1)/UNIFIED_SAMPLING_RATE;	// ms

			lDivR = ldiv(lDivR.rem, 60*60*1000);	// ʱ
			long hhh = lDivR.quot;
			lDivR = ldiv(lDivR.rem, 60*1000);	// ��
			long mmm = lDivR.quot;
			lDivR = ldiv(lDivR.rem, 1000);	// ��
			fprintf(g_pfwInfo, "%05d - %02d:%02d:%02d:%03d(", iSeg+1, hhh, mmm, lDivR.quot, lDivR.rem);
					
			// ��γ�����������
			lDivR.rem = 1000.0*(iSample-iPreEnd)/UNIFIED_SAMPLING_RATE;	// ms
			//
			lDivR = ldiv(lDivR.rem, 60*60*1000);	// ʱ
			hhh = lDivR.quot;
			lDivR = ldiv(lDivR.rem, 60*1000);	// ��
			mmm = lDivR.quot;
			lDivR = ldiv(lDivR.rem, 1000);	// ��
			fprintf(g_pfwInfo, "%02d:%02d:%02d:%03d) ", hhh, mmm, lDivR.quot, lDivR.rem);

			for (int mm=0; mm<500.0*(iSample-iPreEnd)/nNumSampsTotal; mm++) {
				fprintf(g_pfwInfo, "*");
			}
			fprintf(g_pfwInfo, "\n");

			iPreEnd = iSample;	// ��ĩ�����±�
		}	// end of "for (...) {"
		fflush(g_pfwInfo);
	}

	// ���ʣ��Ĳ���һ��֡���Ĳ�������
	return nNumSamplesLeft;
}

void DoSegmentation(const ZONE_VECTOR& zoneVector, int nNumFramesTotal, std::vector<int>& segEndFrmVector)
{
	// ��ʱ�ֶ���Ϣ�ļ���������
	ZONE_VECTOR tmpSegVector;
	ZONE_INFO_T tmpSegInfo;
	tmpSegInfo.iLL = 0;
	tmpSegInfo.iRR = nNumFramesTotal-1;
	tmpSegVector.push_back(tmpSegInfo);
//
	ZONE_INFO_T tmpSegInfo02;
	for (ZONE_VECTOR::const_iterator zzz=zoneVector.begin(); zzz!=zoneVector.end(); zzz++) {
	// ��һ����ѡ�з�������������

		for (ZONE_VECTOR::iterator seg=tmpSegVector.begin(); seg!=tmpSegVector.end(); seg++) {
		// ��һ���ִ�Σ���������
			if (zzz->iLL < seg->iLL || seg->iRR < zzz->iRR) continue;
			if (zzz->iRR - seg->iLL < CCC_010*NumFrameSkipsInUnitM) continue;
			if (seg->iRR - zzz->iRR + 1 < CCC_010*NumFrameSkipsInUnitM) continue;
//
			// �ɷ֣���һ��Ϊ����������
			tmpSegInfo02 = *seg;
			tmpSegVector.erase(seg);
//
			tmpSegInfo.iLL = tmpSegInfo02.iLL;
			tmpSegInfo.iRR = zzz->iRR-1;
			tmpSegVector.push_back(tmpSegInfo);
//
			tmpSegInfo.iLL = zzz->iRR;
			tmpSegInfo.iRR = tmpSegInfo02.iRR;
			tmpSegVector.push_back(tmpSegInfo);

			break;
		}
	}
//
	for (ZONE_VECTOR::const_iterator seg=tmpSegVector.begin(); seg!=tmpSegVector.end(); seg++) {
		segEndFrmVector.push_back(seg->iRR);
	}
}


#endif	// __TRAINING_PHASE

/*
// A return value of -1 indicates an error
int OpenFileWr(const char* psfn);
int OpenFileRd(const char* psfn);

// "psfn" Ϊ����·��
#ifdef __TRAINING_PHASE
int CProcessingObj::ProcessExample(const char *psfn, int iClassNo)
#else
// ���ಢ�ֶΣ�������
int CProcessingObj::ProcessExample(const char *psfn, SEGMENT_VECTOR& segVector)
#endif
{
	int nReturn = CreateSampleDataFile(psfn);
	if (nReturn < 0) {
		return nReturn;
	}

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int& fhandle = nReturn;
	// ��Ŀ���źŲ��������ļ���Ϊ���ź����ݣ�������
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM);
	fhandle = OpenFileRd(m_szfn);
#ifdef __TRAINING_PHASE
	assert(iClassNo > 0);
	m_iClassNo = iClassNo;
//
	if (ProcessTrainingExample(fhandle) != 0) {
		printf("Invalid file %s(ClassNo%02d) !\n", psfn, iClassNo);
//		fprintf(, "Invalid file %s(ClassNo%02d) !\n", psfn, iClassNo);
	}
#else	// __TRAINING_PHASE
	if (SegmentAudio(fhandle, segVector) != 0) {
	}
#endif	// __TRAINING_PHASE
	_close(fhandle);
//
	// ɾ����ʱ���ɵ��ź������ļ�
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM);
	remove(m_szfn);

	return 0;	// 0 : OK; non 0 : not OK
}

int CProcessingObj::CreateSampleDataFile(const char *psfn)
{
	assert(m_pfWaveform);

// 9999ertgjkll
//
	if ( m_pfWaveform->OpenWaveFile(psfn) < 0 ) {
		return ERROR_NO_WAVE_FORM_FILE;
	}

// �����ļ�����ʱ����Ŀ����Ƶ��������
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM);
	int fhandle = OpenFileWr(m_szfn);
	if (fhandle == -1) {
		printf("Can't create file \"%s\" !\n", m_szfn);
		m_pfWaveform->CloseWaveFile();		
		return ERROR_NO_TMP_SAMPLE_DATA_FILE;
	}

	int maxAbs;
	int nReturn = m_pfWaveform->MakeTargetSamplesData(fhandle, maxAbs);	// ����Ŀ���źŲ�������
	_close(fhandle);
	m_pfWaveform->CloseWaveFile();
//
	if (nReturn < 0) {// ����Ŀ���źŲ�������ʱ���ִ���ɾ����ʱ�ļ�
		sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM);
		remove(m_szfn);
		return ERROR_NO_TMP_SAMPLE_DATA_FILE;
	}

	return 0;
}
*/

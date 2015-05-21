

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

#ifndef __TRAINING_PHASE

extern FILE* g_pfwInfo;

int JoinSegments(SEG_INFO_T* pSegInfo, int& nNumSegs, int lenMin);

//

//#define __PIN_POINT_THE_BOUNDARY
#ifndef __PIN_POINT_THE_BOUNDARY
//#define __PIN_POINT_THE_BOUNDARY_02
#endif	// __PIN_POINT_THE_BOUNDARY

#ifdef __PIN_POINT_THE_BOUNDARY
unsigned long PinPoint(const short xxx[]);
#define NUM_FRAME_SKIPS_OF_SMAPLES_READ		NumFrameSkipsInUnitSkipM
#endif	// __PIN_POINT_THE_BOUNDARY
#ifdef __PIN_POINT_THE_BOUNDARY_02
unsigned long StePinPoint(const double ste[]);
#define NUM_FRAME_SKIPS_OF_STE_READ			NumFrameSkipsInUnitM
#endif	// __PIN_POINT_THE_BOUNDARY

//

int CProcessingObj::FinePosition(int fhr_samples, SEG_INFO_T* pSegInfo, int nNumSegs)
{
// ���ģ�
#ifdef __PIN_POINT_THE_BOUNDARY
	// �������ݻ���
	// ÿ����Ԫ��֡������ÿ��֡���Ĳ�������
	short* psamples = Malloc(short, NumFrameSkipsInUnitM*NumSamplesInFrameSkipM);	// ��ʵ��ʵ����Ҫ�Ķ���
	if (psamples == NULL) {
		printf("psamples == NULL !\n");
		return -2;
	}

	// ָ���Ƶ��ļ�ͷ
	_lseek(fhr_samples, 0, SEEK_SET);
	// �ź������ļ���ͷ�� 2 ������
#ifdef _DEBUG
	int sr;
	unsigned long lstart, num_samples;
	if (_read(fhr_samples, &sr, sizeof(int)) != sizeof(int)) {// ������
	}
	if (_read(fhr_samples, &lstart, sizeof(unsigned long)) != sizeof(unsigned long)) {
	}
	if (_read(fhr_samples, &num_samples, sizeof(unsigned long)) != sizeof(unsigned long)) {// ���������
	}
#else
	_lseek(fhr_samples, sizeof(int)+sizeof(unsigned long)+sizeof(unsigned long), SEEK_SET);	// ֱ����������
#endif
	int& sample_ptr = iSeg02;
	sample_ptr = 0;	// �����ļ��е�ǰԪ�ص�λ�ã�Ԫ�ص��±꣩
#endif	// #ifdef __PIN_POINT_THE_BOUNDARY
#ifdef __PIN_POINT_THE_BOUNDARY_02
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_STE);
	FILE *pfrSTE = fopen(m_szfn, "rb");	// ��Ϊ�����ݣ�������

	double* pSTE = Malloc(double, NUM_FRAME_SKIPS_OF_STE_READ);
	if (pSTE == NULL) {
		printf("pSTE == NULL !\n");
		fclose(pfrSTE);
		return -2;
	}
	int& double_ptr = iSeg02;
	double_ptr = 0;	// �����ļ��е�ǰԪ�ص�λ�ã�Ԫ�ص��±꣩
#endif	// __PIN_POINT_THE_BOUNDARY_02

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	int iStartSampleOfNextSeg;
	for (int iSeg=0; iSeg<nNumSegs; iSeg++) {
		if (iSeg == nNumSegs-1) {
			pSegInfo[iSeg].numFrames = pSegInfo[iSeg].end*NumSamplesInFrameSkipM;	// �εĽ���֡����ʼ�������±� 
			pSegInfo[iSeg].numFrames = pSegInfo[iSeg].end+NumSamplesInFrameM;	// ���Ͻ���֡�Ĳ������� 
			pSegInfo[iSeg].numFrames = pSegInfo[iSeg].end-1;	// ��һ������һ���������±�
			break;
		}
		// ��ǰ�ε����һ����Ԫ����ʼ֡�ģ�ȫ�֣��±꣬�����룡��
		iStartSampleOfNextSeg = pSegInfo[iSeg].end-NumFrameSkipsInUnitM+1+
								// �ηֽ紦��������������Ԫ���Ƿ�Χ�����Ĵ�
								(NumFrameSkipsInUnitM+NumFrameSkipsInUnitSkipM)/2;
#ifdef __PIN_POINT_THE_BOUNDARY
		// ���˰����Ԫ��������Ӧ��֡����
		iStartSampleOfNextSeg -= NUM_FRAME_SKIPS_OF_SMAPLES_READ/2;
		iStartSampleOfNextSeg *= NumSamplesInFrameSkipM;	// ת��Ϊ�����±�
		// ���ļ�ָ��λ���Ƶ��±�Ϊ "iStartSampleOfNextSeg" ���źŲ�����λ�� 
		_lseek(fhr_samples, (iStartSampleOfNextSeg-sample_ptr)*sizeof(short), SEEK_CUR);
		// ���� 1 ����Ԫ������Ӧ���źŲ�������
		if (_read(fhr_samples, psamples, NUM_FRAME_SKIPS_OF_SMAPLES_READ*NumSamplesInFrameSkipM*sizeof(short)) != 
										NUM_FRAME_SKIPS_OF_SMAPLES_READ*NumSamplesInFrameSkipM*sizeof(short)) {
			break;
		}
		// 1 ����Ԫ������Ӧ���źŲ�������
		// �ļ�ָ�뵱ǰλ�ö�Ӧ���źŲ������±�
		sample_ptr = iStartSampleOfNextSeg+NUM_FRAME_SKIPS_OF_SMAPLES_READ*NumSamplesInFrameSkipM;

		// ��εľ�ȷ����λ�ã����룩
		iStartSampleOfNextSeg += PinPoint(psamples);
#endif	// #ifdef __PIN_POINT_THE_BOUNDARY
#ifdef __PIN_POINT_THE_BOUNDARY_02
		iStartSampleOfNextSeg -= NUM_FRAME_SKIPS_OF_STE_READ/2;
		fseek(pfrSTE, (iStartSampleOfNextSeg-double_ptr)*sizeof(double), SEEK_CUR);
		if (fread(pSTE, sizeof(double), NUM_FRAME_SKIPS_OF_STE_READ, pfrSTE) != NUM_FRAME_SKIPS_OF_STE_READ) {
			break;
		}
		double_ptr = iStartSampleOfNextSeg+NUM_FRAME_SKIPS_OF_STE_READ;

		iStartSampleOfNextSeg *= NumSamplesInFrameSkipM;	// ֡�±�ת��Ϊ�����±�
		iStartSampleOfNextSeg += StePinPoint(pSTE);
#endif	// #ifdef __PIN_POINT_THE_BOUNDARY_02
//
		pSegInfo[iSeg].end = iStartSampleOfNextSeg-1;	// �ν���λ�ôӴ�ǰ��֡�±��Ϊ�źŲ������±꣬�����á�
	}
#ifdef __PIN_POINT_THE_BOUNDARY
	free(psamples);	// �������ݻ���
	psamples = NULL;
#endif	// __PIN_POINT_THE_BOUNDARY
#ifdef __PIN_POINT_THE_BOUNDARY_02
	fclose(pfrSTE);
	free(pSTE);
#endif	// __PIN_POINT_THE_BOUNDARY_02

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// �����̵Ķκϲ������ڶ���
	nNumSegsDeleted = JoinSegments(pSegInfo, nNumSegs, MINMUN_NUM_FRAMES_IN_SEGMENT);
	if (g_pfwInfo) {
		fprintf(g_pfwInfo, "\n���շֶ���Ϣ��������\n");
		fprintf(g_pfwInfo, "\n�ϲ���ʣ�� %d �Σ���ȥ�� %d �����̶Σ���������\n", nNumSegs, nNumSegsDeleted);
	}
*/
	return 0;
}

#ifdef __PIN_POINT_THE_BOUNDARY_02

// �������е㣬������
// "ste[]" ��ǰ��Ԫ��unit����ǰ "NumFrameSkipsInUnitSkipM" ֡����Ӧ���ź�
unsigned long StePinPoint(const double ste[])
{
	int mm = NUM_FRAME_SKIPS_OF_STE_READ/2;	// ����֡
	if (ste[mm] <= MINIMUM_MEAN_ABS_SIGNAL_VALUE) {
		return mm*NumSamplesInFrameSkipM;
	}
	int ll = mm-1;	// ����
	int rr = mm+1;	// ����
	while (ll >= 0 || rr < NUM_FRAME_SKIPS_OF_STE_READ) {
		if (ll >= 0) {
			if (ste[ll] <= MINIMUM_MEAN_ABS_SIGNAL_VALUE) {
				return ll*NumSamplesInFrameSkipM;	// �������ģ���
			}
			ll--;
		}
		if (rr < NUM_FRAME_SKIPS_OF_STE_READ) {
			if (ste[rr] <= MINIMUM_MEAN_ABS_SIGNAL_VALUE) {
				return rr*NumSamplesInFrameSkipM;	// �������ģ���
			}
			rr++;
		}
	}

	return mm*NumSamplesInFrameSkipM;	// û�취��
}

#endif	// #ifdef __PIN_POINT_THE_BOUNDARY_02

#ifdef __PIN_POINT_THE_BOUNDARY

// �������е㣬������
// "xxx[]" ��ǰ��Ԫ��unit����ǰ "NumFrameSkipsInUnitSkipM" ֡����Ӧ���ź�
// ���ź�������С��֡
unsigned long PinPoint(const short xxx[])
{
	int isampleG = 0;
	double dbmin = DBL_MAX;
	unsigned long imin;
	double dbsum;
	for (int ifrm=0; ifrm<NUM_FRAME_SKIPS_OF_SMAPLES_READ; ifrm++) {// 1 ����Ԫ������Ӧ��֡��������
		dbsum = 0.0;
		for (int ii=0; ii<NumSamplesInFrameSkipM; ii++) {
			dbsum += abs(xxx[isampleG]); isampleG++;
		}
		if (dbmin > dbsum) {
			dbmin = dbsum; imin = ifrm;
		}
	}
	imin *= NumSamplesInFrameSkipM;	// ��֡�±����źŲ�������±꣨���ֵ����ǰ��Ԫ�ڣ�
	return imin;
}

#endif	// #ifdef __PIN_POINT_THE_BOUNDARY

#endif	// #ifndef __TRAINING_PHASE


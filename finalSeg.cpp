

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
// ��ÿ������Ρ���������һ���������
int CProcessingObj::FinalSegProcessing(int fhr_samples, int& nNumSamplesCur, int nNumSamplesLeft, SEGMENT_VECTOR& segVector)
{
	// ��ŶεĽ���λ�ã���Ԫ��ţ�
	SEG_INFO_T* pSegInfo = Malloc(SEG_INFO_T, m_unitVector.size());	// �Դ��ֶε���Ƶ�е�Ԫ�ĸ���Ϊ����
	if (pSegInfo == NULL) {
		printf("No memory, pSegInfo == NULL !\n");
		return -1;
	}

// ��һ���Ե�Ԫ����ͬ��ϲ���������ͬ��ϲ���������Ȼ�Σ�������

	int nNumSegs = 0;	// ��Ȼ����
	int iClassNoPre = -1;
	for (SEG_VECTOR_L::size_type iUnit=0; iUnit<m_unitVector.size(); iUnit++) {
		if (m_unitVector[iUnit].classNo != iClassNoPre) {// �¶ο�ʼ����һ��Ҳ�ǣ�
			pSegInfo[nNumSegs].classNo = m_unitVector[iUnit].classNo;
			pSegInfo[nNumSegs].numFrames = m_unitVector[iUnit].numFrames;	// ���һ��ᱻ�޸�
			nNumSegs++;

			iClassNoPre = m_unitVector[iUnit].classNo;
		} else {// ��ǰ��ͬ�࣬����ǰ�Σ�
			pSegInfo[nNumSegs-1].numFrames += m_unitVector[iUnit].numFrames;	// ע���±꣬"nNumSegs-1" !!!
		}
	}
//
	m_unitVector.clear();	// ֻΪ�ͷ��ڴ�


	if (g_pfwInfo) {
		fprintf(g_pfwInfo, "���� %d ����Ȼ�Σ�������\n", nNumSegs);
	}
	assert(nNumSegs > 0);
	pSegInfo = (SEG_INFO_T*)realloc(pSegInfo, sizeof(SEG_INFO_T)*nNumSegs);	// �ͷŶ����ڴ�

/*	// ����м�����ֻΪ���Գ���
	if (g_pfwInfo) {
		for (int iSeg=0; iSeg<nNumSegs; iSeg++) {
			fprintf(g_pfwInfo, "UnitNo - %05d : ClassNo - %02d\n", pSegInfo[iSeg].end, pSegInfo[iSeg].classNo);
		}
	}*/

//
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// �����������̵Ķβ������ڶ�

	int nNumSegsDeleted;
	nNumSegsDeleted = JoinSegments(pSegInfo, nNumSegs, MINMUN_NUM_FRAMES_IN_SEGMENT);
	if (g_pfwInfo) {
		fprintf(g_pfwInfo, "���̵Ķκϲ���ʣ�� %d �Σ���ȥ�� %d ����Ȼ�Σ���������\n", nNumSegs, nNumSegsDeleted);
	}

// ��������������ֵ�ľ����κϲ������ڶ��У�������

	int iSeg;
	int iSegDest = 0;
	for (iSeg=0; iSeg<nNumSegs; iSeg++) {
		if (iSeg == 0) {// ��һ�Σ������Ƿ����Σ����Ǳ�����������
			// �����Ǿ����Σ������������ڶΣ��ʱ���
			pSegInfo[iSegDest].classNo = pSegInfo[iSeg].classNo;
			pSegInfo[iSegDest].numFrames = pSegInfo[iSeg].numFrames;
			iSegDest++;

			iClassNoPre = pSegInfo[iSeg].classNo;
			continue;	// !!!
		}

		// ֻ�е����ڶ���������һ�������κ󣬲ſ��ܳ�������������������ͬ���֮�����һ�������Σ�
		if (pSegInfo[iSeg].classNo == iClassNoPre) {// �������ڶΣ�
			pSegInfo[iSegDest-1].numFrames += pSegInfo[iSeg].numFrames;	// ע���±꣬"iSegDest-1" !!!
			continue;	// !!!
		}

		if (pSegInfo[iSeg].classNo == 0 && pSegInfo[iSeg].numFrames < NUM_FRAMES_IN_SILENT_SEGMENT_TO_SURVIVE) {
		// �Ƚ϶̵ľ����Σ��������ڶΣ�������
			pSegInfo[iSegDest-1].numFrames += pSegInfo[iSeg].numFrames;	// ע���±꣬"iSegDest-1" !!!
		} else {// ���Ǿ����Σ����ǽϳ��ľ����Σ�������������			
			pSegInfo[iSegDest].classNo = pSegInfo[iSeg].classNo;
			pSegInfo[iSegDest].numFrames = pSegInfo[iSeg].numFrames;
			iSegDest++;

			iClassNoPre = pSegInfo[iSeg].classNo;
		}
	}
	if (g_pfwInfo) {
		fprintf(g_pfwInfo, "�����κϲ���ʣ�� %d �Σ������� %d �Σ���������\n", iSegDest, nNumSegs-iSegDest);
	}
	nNumSegs = iSegDest;	// �޸Ķ���

//	
// ע�⣺ÿ�ε�֡���Ǵ�ǰһ�εĽ���֮֡���֡�����ν���֡��֡����
//			��Ȼ����һ�ε���ʼ֡�ǵ� 0 ֡��

	// λ��֡��
	int& nOffsetFrames = nNumSegsDeleted;
	nOffsetFrames = NumFrameSkipsInUnitM-(NumFrameSkipsInUnitSkipM+NumFrameSkipsInUnitM)/2;
	int& iSample = iSegDest;
	// �ȽϺ���Ķμ��зֵ������ڵ�Ԫ����ǰ�εĽ�����Ԫ�ͺ�εĿ�ʼ��Ԫ�����Ƿ�Χ�����ĵ㣬������
	int nNumFrames = -nOffsetFrames;
	for (iSeg=0; iSeg<nNumSegs; iSeg++) {
		nNumFrames += pSegInfo[iSeg].numFrames;	// ������ĩβ֡Ϊֹ����֡��
		nNumFrames += (iSeg == nNumSegs-1)?nOffsetFrames:0;	// λ��֡���ز��������ܳ��ͻ᲻����
		iSample = (nNumFrames-1)*NumSamplesInFrameSkipM+NumSamplesInFrameM;	// ��֡������ɲ�������������
		if (iSeg == nNumSegs-1) {// �Ǳ�����Ρ������һ�Σ�
			iSample -= NumSamplesInFrameM-NumSamplesInFrameSkipM;	// �۳�һ���֣�������һ����Ρ�
			// ע�⣺ֻ�ж����һ������Ρ���"nNumSamplesLeft" ��ֵ�Ų�Ϊ 0
			iSample += nNumSamplesLeft;	// ����ʣ��Ĳ���һ֡�����Ĳ�������
		}
//
		iSample += nNumSamplesCur;	// ������ǰ���С���Ρ��ۼƵĲ�����
		if (iSeg == nNumSegs-1) {// �Ǳ�����Ρ������һ�Σ�
			// �����µ���ǰ���С���Ρ��ۼƲ��������Ա������һ����Ρ�ʱ��
			nNumSamplesCur = iSample;
		}
//
		// ���������š��͡��ν���λ�ã����룩�������������
		segVector.push_back(CSegment(pSegInfo[iSeg].classNo, 1000.0*(iSample-1)/UNIFIED_SAMPLING_RATE));
	}

	free(pSegInfo);

	return 0;
}

#endif	// #ifndef __TRAINING_PHASE


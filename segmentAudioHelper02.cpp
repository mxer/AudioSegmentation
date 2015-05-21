

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

void STE(const short xxx[], int NNN, double& ste);

#define NumShortsBufferCanHold	(1023*NumSamplesInFrameSkipM+NumSamplesInFrameM)	// 1024 ֡

// �����������֡�� STE ֵ����д����ʱ�ļ����������ʣ��Ĳ���һ֡�����Ĳ�������
int CProcessingObj::SegmentAudioHelper02(int fhr)
{
	short* pBuf = Malloc(short, NumShortsBufferCanHold);	// ��������
	if (pBuf == NULL) {
		printf("No memory, pBuf == NULL !\n");
		return -1;
	}
//
	// ������ʱ�ļ���������
	FILE* pfwSTE;
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_STE);
	pfwSTE = fopen(m_szfn, "wb");	// �����ļ���
	if (pfwSTE == NULL) {
		printf("pfwSTE == NULL, Can't create file !\n");
		free(pBuf);
		return -2;
	}
//
	int nNumFrames = 0;
	if (fwrite(&nNumFrames, sizeof(int), 1, pfwSTE) != 1) {// ռλ
	}
//
// �ź������ļ�ͷ�ϵ� 3 ������
	_lseek(fhr, 0, SEEK_SET);	// �ļ�ͷ
#ifdef _DEBUG
	int sr;
	unsigned long lstart, num_samples;
	if (_read(fhr, &sr, sizeof(int)) != sizeof(int)) {// ������
	}
	if (_read(fhr, &lstart, sizeof(unsigned long)) != sizeof(unsigned long)) {
	}
	if (_read(fhr, &num_samples, sizeof(unsigned long)) != sizeof(unsigned long)) {// ���������
	}
#else
	_lseek(fhr, sizeof(int)+sizeof(unsigned long)+sizeof(unsigned long), SEEK_SET);	// ֱ����������
#endif
//
	double dbSTE;
	int nNumShorts = 0;
	short* pCurFrame;
	while (1) {
		int nBytesRead = _read(fhr, pBuf+nNumShorts, (NumShortsBufferCanHold-nNumShorts)*sizeof(short));
		if (nBytesRead <= 0) break;

		nNumShorts += nBytesRead/sizeof(short);	// ԭ�еļ����¶���ģ��ֽ��� --> ����������
		pCurFrame = pBuf;
		while (nNumShorts >= NumSamplesInFrameM) {// ��һ֡��������һ֡�������ˣ�
			STE(pCurFrame, NumSamplesInFrameSkipM, dbSTE);	// ֻ����һ֡����Ӧ�Ĳ�����֡�� STE ֵ���㣡
			if (fwrite(&dbSTE, sizeof(double), 1, pfwSTE) != 1) {
				break;
			}
			nNumFrames++;
		//
			nNumShorts -= NumSamplesInFrameSkipM;	// δ����Ĳ�������
			pCurFrame += NumSamplesInFrameSkipM;	// δ����Ĳ�����ʼλ��
		}
		// ʣ�µĲ���һ֡�Ĳ���
		memmove(pBuf, pCurFrame, nNumShorts*sizeof(short));
	}
	free(pBuf);
//
	fseek(pfwSTE, 0, SEEK_SET);
	if (fwrite(&nNumFrames, sizeof(int), 1, pfwSTE) != 1) {
	}
	fclose(pfwSTE);
//
	nNumShorts -= NumSamplesInFrameM-NumSamplesInFrameSkipM;

	return nNumShorts;	// ���ʣ��Ĳ���һ��֡���Ĳ�������
}

void CProcessingObj::JoinSameClassSegs(SEGMENT_VECTOR& segVector)
{
	int nClassNoPre = -1;	// ���������������
	SEGMENT_VECTOR::iterator itrPre;
//
	SEGMENT_VECTOR::iterator itr = segVector.begin();
	while (itr != segVector.end()) {
		if (itr->nClassNo == nClassNoPre) {// �����ͬ�࣬������
			itrPre->lSegEnd = itr->lSegEnd;	// �������
			itr = segVector.erase(itr);	// ɾ�����Σ���Ȼ������һ��Ԫ�أ�itr��
			continue;
		}

		// ����β�ͬ�࣬����
		nClassNoPre = itr->nClassNo;
		itrPre = itr;

		itr++;	// ������һ��Ԫ�أ�itr��
	}
}

void CProcessingObj::SaveSegInfoToFile(SEGMENT_VECTOR& segVector)
{
	if (g_pfwInfo == NULL) return;

	fprintf(g_pfwInfo, "\n���շֶ���Ϣ���� %d �Σ�������\n", segVector.size());

	long nLenTotal = segVector.back().lSegEnd+1;	// �ܳ������룩

	short hhh, mmm;	// ʱ����
	ldiv_t lDivR;
	long lEndPre = -1;
	for (SEGMENT_VECTOR::size_type iSeg=0; iSeg<segVector.size(); iSeg++) {
		// �ν���λ�ã������±��Ӧ��ʱ�䣩
		lDivR.rem = segVector[iSeg].lSegEnd;	// ms

		lDivR = ldiv(lDivR.rem, 60*60*1000);	// ʱ
		hhh = lDivR.quot;
		lDivR = ldiv(lDivR.rem, 60*1000);	// ��
		mmm = lDivR.quot;
		lDivR = ldiv(lDivR.rem, 1000);	// ��
		fprintf(g_pfwInfo, "%05d - %02d:%02d:%02d:%03d(", iSeg+1, hhh, mmm, lDivR.quot, lDivR.rem);

		// ��γ�����������
		lDivR.rem = segVector[iSeg].lSegEnd-lEndPre;
//
		lDivR = ldiv(lDivR.rem, 60*60*1000);	// ʱ
		hhh = lDivR.quot;
		lDivR = ldiv(lDivR.rem, 60*1000);	// ��
		mmm = lDivR.quot;
		lDivR = ldiv(lDivR.rem, 1000);	// ��
		fprintf(g_pfwInfo, "%02d:%02d:%02d:%03d) ", hhh, mmm, lDivR.quot, lDivR.rem);
		fprintf(g_pfwInfo, "ClassNo%02d ", segVector[iSeg].nClassNo);

		for (int mm=0; mm<500.0*(segVector[iSeg].lSegEnd-lEndPre)/nLenTotal; mm++) {
			fprintf(g_pfwInfo, "*");
		}
		fprintf(g_pfwInfo, "\n");

		lEndPre = segVector[iSeg].lSegEnd;	// ms
	}
}

#endif	// __TRAINING_PHASE


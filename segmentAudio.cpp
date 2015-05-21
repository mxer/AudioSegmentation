

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

#include "svmInterface.h"

// "fhr_samples" �����ļ������ֶε���Ƶ�ź��ļ�
int CProcessingObj::SegmentAudio(int fhr_samples, SEGMENT_VECTOR& segVector)
{
	// ÿ���ε�ĩβ֡������֡���±�
	std::vector<int> initSegEndVector;
	// ����ֵ��ʣ��Ĳ���һ֡���Ĳ�������
	int nNumSamplesLeft = SegmentAudio02(fhr_samples, initSegEndVector);
	if (nNumSamplesLeft < 0) {
		return -1;
	}
	if (initSegEndVector.back()+1 < NumFrameSkipsInUnitM) {// "initSegEndVector.back()+1" ��֡����
		printf("Length less than a unit, abort !\n");
		return -2;
	}
// SVM things
	CSvmInterface* pSvm = new CSvmInterface(m_psInPath, m_psOutPath);
	if (pSvm == NULL || pSvm->LoadSvmModel(m_nNumDims) != 0) {
		return -3;
	}

// �����С�����÷��¼���һ����Ԫ������źŲ�������
	int nBufSizeInShorts = NumSamplesInFrameSkipM*(NumFrameSkipsInUnitM-1)+NumSamplesInFrameM;
	short* psampd = (short*)malloc(sizeof(short)*nBufSizeInShorts);
	if (psampd == NULL) {
		delete pSvm;
		return -4;
	}

// �ź������ļ�ͷ�ϵ� 3 ������
	_lseek(fhr_samples, 0, SEEK_SET);	// �ļ�ͷ
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

	SEG_INFO_T segInfo;
	short* pCurFrame;
	int nNumUnits;
	int nShorts = 0;	// a must !
	int iFrmG = 0;	// ����֮֡ȫ���±�
	int nNumSamplesCur = 0;	// �Ѵ���Ĵ���в��������ۼ�
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// Լ������֡����һ����Ԫ��unit������ÿ����Ԫ���з��ࣻ
// ��Լ������Ԫ��������ص�������ӣ��������룡����
	for (std::vector<int>::size_type isub=0; isub<initSegEndVector.size(); isub++) {
		// ������ʱ�ļ���������
		sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_STE);
		m_pfSTE = fopen(m_szfn, "wb");	// �����ļ���
		if (m_pfSTE == NULL) {
			printf("m_pfSTE == NULL, Can't create file !\n");
		}
		m_unitVector.clear();	// �����
//
		m_nNumFrames = 0;	// �ӵ�ǰ��������ȡ��֡����������Ԫ��֡��������
		nNumUnits = 0;	// ��ǰ��Ƶ�е�Ԫ����
		while (1) {
			int nBytesRead = _read(fhr_samples, psampd+nShorts, (nBufSizeInShorts-nShorts)*sizeof(short));
			if (nBytesRead <= 0 && nBufSizeInShorts > nShorts) {// û�в������ݿɶ��ˣ�������ˣ��˳���������
				break;
			}
//
			nShorts += nBytesRead/sizeof(short);	// �����в�����short��������ԭ�еļ��¶��ģ�������
			pCurFrame = psampd;	// ������ַ
			while (nShorts >= NumSamplesInFrameM) {// �����д�����Ĳ���������һ֡��������
				if (iFrmG > initSegEndVector[isub]) {// ��ǰ��ε�֡�����������������������
					memmove(psampd, pCurFrame, nShorts*sizeof(short));
					goto PROC_SECTION;
				}
				ProcSingleFrame(pCurFrame);	// ����һ֡�ź�����ش���
				m_nNumFrames++;	// ֡������
				iFrmG++;
//
				if (m_nNumFrames == NumFrameSkipsInUnitM) {// �����֡����һ����Ԫ�ˣ���������ȴ���������
					int iClassNo;
					// ���ɵ�ǰ unit ������������������
					if (MakeFeatureVector() == 0) {// �������� OK��������
						// ��ʶ��ǰ unit ������� unit Ϊ��λ�����ࣩ��������
						iClassNo = pSvm->ppsvmPredict(m_pvector, m_nNumDims);
					} else {// �������� NOT OK��������
						iClassNo = 0;
					}
					segInfo.classNo = iClassNo;
//
					segInfo.numFrames = NumFrameSkipsInUnitSkipM;
					if (nNumUnits == 0) {
						segInfo.numFrames = NumFrameSkipsInUnitM;
					}
//
					m_unitVector.push_back(segInfo);	// ��Ԫ��β֡���±�
					nNumUnits++;
/*					if (g_pfwInfo) {// "g_pfwInfo" ����Ϊ�գ������Ͳ�����ļ���
						fprintf(g_pfwInfo, "iUnit : %05d[%09.3f - ], iClassNo : %02d\n", 
								m_nNumUnits-1, 20.0*NumFrameSkipsInUnitSkipM*(m_nNumUnits-1)/1000.0, iClassNo);
					}*/
//
// ��ԽС�� "NumFrameSkipsInUnitSkipM" ֵ���ν�λ�þ���Խ�ߣ�������
// ��Ҫ�������õġ�֡��������������Ϊ "NumFrameSkipsInUnitM-NumFrameSkipsInUnitSkipM"

					MoveUnitOverlap();
				}	// end of "if (m_nNumFrames == NumFrameSkipsInUnitM) {"

				pCurFrame += NumSamplesInFrameSkipM;	// ��һ����֡��������һ֡�ź���ַ
				nShorts -= NumSamplesInFrameSkipM;	// �������źŲ�����������

				if (nNumUnits == MAX_NUM_UNITS) {// �ڴ������ˣ�����ĵ�Ԫ�������ˣ�������
					printf("iUnit == MAX_NUM_UNITS !\n");
					break;
				}
			}	// end of "while (nShorts >= NumSamplesInFrameM) {// �����д�����Ĳ���������һ֡��������"

			if ((nBytesRead % sizeof(short)) != 0) {// �ļ��д�����
				printf("Sample data invalid !\n");
				break;
			}

			// ʣ�µĴ������������һ֡�ˣ������Ƶ������ף����������ٴ���������
			memmove(psampd, pCurFrame, sizeof(short)*nShorts);
//
			if (nNumUnits == MAX_NUM_UNITS) {
				break;
			}
		}	// end of "while (1) {"

PROC_SECTION:
		// ��ǰ��ε����һ����Ԫ�Ľ���֡���±꣬��Ҫ������������
		segInfo = m_unitVector.back();	// ȡ���һ����Ԫ��ֵ
		m_unitVector.pop_back();	// ɾ�����һ����Ԫ
		// ���ϲ���һ����Ԫ������ʣ��֡������֡����
		segInfo.numFrames += m_nNumFrames-(NumFrameSkipsInUnitM-NumFrameSkipsInUnitSkipM);
		m_unitVector.push_back(segInfo);
//
		fclose(m_pfSTE);	// �ر��´������ļ�
		m_pfSTE = NULL;

		//////////////////////////////////////////////////

		if (g_pfwInfo) {
			fprintf(g_pfwInfo, "\n����� %d ��Σ�������\n", isub+1);
		}
		
		// "nNumSamplesLeft" ����һ֡�����Ĳ�������
		int& nNumberOfSamples = nNumUnits;
		nNumberOfSamples = 0;
		if (isub == initSegEndVector.size()-1) {// �����һ������Ρ�
			nNumberOfSamples = nNumSamplesLeft;
			nNumberOfSamples += NumSamplesInFrameM-NumSamplesInFrameSkipM;	// ��ӣ����۳���������
		}
		// �����շֶΣ�������ȷ�Ķηֽ�㣩
		FinalSegProcessing(fhr_samples, nNumSamplesCur, nNumberOfSamples, segVector);

		sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_STE);
		remove(m_szfn);
	}	// end of "for () {"
//
//////////////////////////////////////////////////////////////////////////////////

	free(psampd);
	delete pSvm;
// �ͷ������������õ��ڴ�
//	ReleaseMemory();	// ��Ϊ���ͷ��ڴ棬��Ϊ���������Զ����Ƶ�ļ����зֶΣ�����

	return 0;
}

#endif	// #ifndef __TRAINING_PHASE


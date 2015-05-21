
#include "stdafx.h"

#include <assert.h>
#include <FLOAT.H>	// for DBL_MAX

#include <math.h>
#include <stdio.h>
#include <io.h>

#include <stdio.h>
#include <malloc.h>

#include <string.h>

#include "parameters.h"

#include "main_seg.h"
#include "my_util.h"

extern const char *g_psOutRootPath;

//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#define __CUT_SMALL_HEAD_AND_TAIL

// head_tail_cut_001
#ifdef __CUT_SMALL_HEAD_AND_TAIL
// ���ҵ���ƽ���������Ķ�֮��Ϊ����������ε�ͷ���Һ�β������ܵ��������֣�������
// �ź�ֵ�ľ���ֵƽ��������ô������ skips �ĸ����ƣ��Ĵ�����Ϊ��������ⴰ�����м����
#define	NUM_SKIPS_IN_DETECT_WIN02			2		// accounts for 0.06 seconds
#endif
// head_tail_cut_001


// ��ⴰ�ڵĴ�С��������趨���������������ƣ�ÿ�����̶�Ϊ 30 ���룩��
#define DetectWinSizeInSkips		m_usMaxMainSegSizeInSkips	// ����

// ���ֵ��������С���� double ֵ�����ƣ��������ܷ��� "DetectWinSizeInSkips" �����ֵ 
// ÿ�����ֵ�Ǹ� double ֵ����Ӧһ����ⴰ������
// ��ⴰ���������ƣ��͵�һϵ�м�ⴰ��ÿ����ⴰ��Ӧһ�����ֵ�������������������������Щ���ֵ��
// ΪʲôҪ���棨��ס�����ֵ�����ⴰλ�ò��͵��ˣ���Ϊ�˼��ֵ��������Ч�ʣ���������ֻ���ļ���һ�Σ�������
#define BufferSizeInDoubles		DetectWinSizeInSkips	// �����

#define STR_TEMP_SAMPS_FILE_NAME	"oss_cb.bin"

CMainSegment::CMainSegment()
{
	// ��ؼ��μ�ⴰ�ڵĿ�ȣ�Ҳ���ؼ��ο�ȣ����Դ������������ƣ������̶�Ϊ 30 ���롣
	m_usMaxMainSegSizeInSkips = 300;	// in skips, 30 ms per skip
//
	// ��ⴰ��������С���̶�Ϊ 30 ����ʱ����Ӧ���źŲ����������롰Ŀ������ʡ��йأ���
	m_usNumSampsInSkip	= (double)UNIFIED_SAMPLING_RATE/1000.0*MILISECONDS_DETECT_SKIP;
	m_pdbData = (double *)malloc(sizeof(double)*BufferSizeInDoubles);

	// ������ "m_pdbData" �еġ���һ�� double ֵ����Ӧ�� skip ��ţ�skip ��Ŵ� 0 ��ʼ��
	m_lSkipIdxG_FirstValInBuffer = 0;
	// ������ "m_pdbData" �еġ���һ��ֵ�����±꣨����ڻ���������ʼλ�ã�
	m_shFirstValIdxInBufferL = 0;

	m_lCurSkipIdxG = 0;
	m_dbValCurSkip = 0.0;
	m_lStartSampIdxG_NextSkip = m_usNumSampsInSkip;

	m_lSkipIdxG_MaxDetectVal = -1;

	m_lNumOriginalSamples = 0;

	// ����ʱ�ļ����ڴ��Ƿ�����������ϲ�����ѡ��һ��֮�����Ƶ�ź����ݡ���ԭʼ��Ƶ���ݶ�Ӧ��ʱ��һ��������
	char *psName = (char*)malloc(260);
	sprintf(psName, "%s%s", g_psOutRootPath, STR_TEMP_SAMPS_FILE_NAME);
	m_pfProcessedSamps = fopen(psName, "wb");	// ����
	free(psName);

	_Xn_Max = 0;
}

CMainSegment::~CMainSegment()
{
	if (m_pdbData)
		free(m_pdbData);

	if (m_pfProcessedSamps) {
		fclose(m_pfProcessedSamps);
	}
	char *psName = (char*)malloc(260);
	sprintf(psName, "%s%s", g_psOutRootPath, STR_TEMP_SAMPS_FILE_NAME);
	remove(psName);	// remove the temp file
	free(psName);
}

/*
void CMainSegment::StartDetection()
{

}
void CMainSegment::DetectionEnd()
{

}
*/

// �ڼ����ؼ��Σ��������������ĶΣ�֮�󣬿��ܻ�Ҫ����ͷȥβ������Ϊͷβ����̫С��
// "pfw_vsd_b" ��������ѡ�������ֹؼ��ε���Ƶ�ź�����
// ������� "*pulStart" �� "*pulNumSamps" �����Ŀ������ʣ�
// 
int CMainSegment::WriteMainSegToFile(int fhw, unsigned long *pulStart, unsigned long *pulNumSamps)
{
	if (m_pfProcessedSamps) { fclose(m_pfProcessedSamps); }

//	printf("\nMaximum signal value : %u\n", _Xn_Max);	// ��ǰ�������ź�ֵ����ֵ���ֵ
//	fprintf(, "\nMaximum signal value : %u\n", _Xn_Max);	// ��ǰ�������ź�ֵ����ֵ���ֵ

// ////////////////////////////////////////////////////////////////////////////////////////////

	char *psName = (char*)malloc(260);
	sprintf(psName, "%s%s", g_psOutRootPath, STR_TEMP_SAMPS_FILE_NAME);
	m_pfProcessedSamps = fopen(psName, "rb");	// ��
	free(psName);

//��һ����ؼ��β�����������������

	int nReturn = 0;
	unsigned long lStartSampleGlobal, lNumValidSamples;
	if (m_lSkipIdxG_MaxDetectVal == -1) {// ��Ƶ�εĳ��Ȳ���һ����ⴰ�ڣ��ؼ��εĳ���Ϊ��Ƶʵ�ʳ���
		lStartSampleGlobal = 0;
		lNumValidSamples = m_lNumOriginalSamples;

	} else {// ��ʱ�ؼ��εĳ���Ϊ��ⴰ�ڿ�ȣ��˼�����ֵ����
		lStartSampleGlobal = m_lSkipIdxG_MaxDetectVal*m_usNumSampsInSkip;
		lNumValidSamples = DetectWinSizeInSkips*m_usNumSampsInSkip;
	}
	if (lNumValidSamples < (MINIMUM_MUSIC_LENGTH_ALLOWED_IN_SECONDS*UNIFIED_SAMPLING_RATE)) {
		printf("Music file too short, %8.4f(s), abort !\n", (double)lNumValidSamples/UNIFIED_SAMPLING_RATE);
		printf("Music file too short, %8.4f(s), abort !\n", (double)lNumValidSamples/UNIFIED_SAMPLING_RATE);
		nReturn = -10; goto ExitHere;
	}

	unsigned long lNeedToReadSize;
	size_t nSizeRead;
	size_t nSizeInShorts = BufferSizeInDoubles*sizeof(double)/sizeof(short);

// head_tail_cut_002
#ifdef __CUT_SMALL_HEAD_AND_TAIL
//����������ؼ���ͷβ����̫С����Ҫ����ͷȥβ���������ڵĴ���������ͷ�£��ó��Ĺؼ��Σ����в�����
//		����������Ҫ�󣨼�����̫С��������

	// ���ļ�ָ���Ƶ��ؼ�����ʼλ��
	fseek(m_pfProcessedSamps, sizeof(short)*lStartSampleGlobal, SEEK_SET);

	// ��ⴰ��02���ɼ��� skip ���ɣ��͵��м�����Ԫ����¼��Щ skips �����ڲ��ź�ֵ�ľ���ֵ֮��
	double dbValSkip[NUM_SKIPS_IN_DETECT_WIN02];
	int iSkip = 0;
	dbValSkip[iSkip] = 0.0;

	int nNumSkips = 0;	// a detection window consists of "NUM_SKIPS_IN_DETECT_WIN02" skips
	int nNumSamples = 0;	// a skip consists of "m_usNumSampsInSkip" samples
	int iPosBuffL;	// point to a position in the local buffer
	int iSampleGlobal = lStartSampleGlobal;
	int bStatus = 0;
	unsigned long iSampleStartG, iSampleStopG;

	// ���� "m_pdbData[]" ������źŲ������ݣ�ÿ��������һ�� "short"��
	// figure out the number of shorts the buffer can hold
	lNeedToReadSize = lNumValidSamples;
	while (!feof(m_pfProcessedSamps) && lNeedToReadSize > 0) {
		nSizeRead = (lNeedToReadSize > nSizeInShorts)?nSizeInShorts:lNeedToReadSize;
		nSizeRead = fread(m_pdbData, sizeof(short), nSizeRead, m_pfProcessedSamps);
		if (nSizeRead == 0) { break; }

		///////////////////////////////////////////////////////////////////////////

		iPosBuffL = 0;
		while (iPosBuffL < nSizeRead) {// ���δ��ļ�����������δ�����꣬������
			dbValSkip[iSkip] += abs(((short *)m_pdbData)[iPosBuffL]);
			iPosBuffL++;	// next sample in the local buffer

			nNumSamples++;
			if (nNumSamples < m_usNumSampsInSkip) {// �� skip ��Ĳ��������������������
				continue;
			}

			// һ�� skip ���ˣ�������

			nNumSkips++;	// skip ��������
			if (nNumSkips == NUM_SKIPS_IN_DETECT_WIN02) {// ��һ����ⴰ��������ⴰ������������ֵ
				// ����ⴰ������ skip ��ֵ�ۼ�����
				double dbValWindow = 0.0;
				for (int ii=0; ii<NUM_SKIPS_IN_DETECT_WIN02; ii++) {
					dbValWindow += dbValSkip[ii]; 
				}
				dbValWindow /= NUM_SKIPS_IN_DETECT_WIN02*m_usNumSampsInSkip;

				if (bStatus == 0) {// ������δ�ҵ���ʼ�㣬������
					if (dbValWindow >= MINIMUM_MEAN_ABS_SIGNAL_VALUE) {// ��������Ҫ��ļ�ⴰ��������
						bStatus = 1;	// ��ʾ�ҵ���ʼ����
						// ����ⴰ��Ӧ�ĵ�һ���������λ��
						iSampleStartG = iSampleGlobal + iPosBuffL-m_usNumSampsInSkip*NUM_SKIPS_IN_DETECT_WIN02;
						// ����ⴰ��Ӧ�����һ���������λ��
						iSampleStopG = iSampleGlobal + iPosBuffL-1;
					}
				} else {// ��ʼ�����ҵ������ǵ�һ������Ҫ��ļ�ⴰ����������
				// ����Ĵ�����ģ���˴�β����ɨ�裬ֱ���������ʵļ�ⴰλ��
					if (dbValWindow >= MINIMUM_MEAN_ABS_SIGNAL_VALUE) {
					// ����λ�ü��ɡ����ռǵ������һ������Ҫ��ļ�ⴰλ�õ����һ���������λ��
						iSampleStopG = iSampleGlobal + iPosBuffL-1;
					} else {
					// �м������˲�����Ҫ��ļ�ⴰ�������ܾʹ˽ضϣ���Ϊ������ܻ��з���Ҫ��ļ�ⴰ��������������
					}
				}

				// ��˼��˵���ڼ�ⴰ�� skip ���ֵ�����У����������Զ�� skip �ļ��ֵ�����ˣ����໹���ã���
				// ���� skip �����ټ�һ��
				nNumSkips--;
			}

			iSkip++;	// �ڼ�ⴰ�� skip ���ֵ�����У��ĸ���Ԫ����������һ��������� skip �ļ��ֵ
			if (iSkip == NUM_SKIPS_IN_DETECT_WIN02) {
				iSkip = 0;	// skip ���ֵ�����Ǹ�ѭ�� buffer ��
			}
			dbValSkip[iSkip] = 0.0;	// ����׼���ۼ�
			nNumSamples = 0;	// ��һ�� skip �еĲ������������
		}

		////////////////////////////////////////////////////////////////

		iSampleGlobal += nSizeRead;	// ���ػ����׵�ַ��Ӧ��ȫ�ֲ��������
		lNeedToReadSize -= nSizeRead;	// decrease
	}

	if (bStatus == 0) {// ���м�ⴰλ�ö�����Ҫ��
		lNumValidSamples = 0;
		printf("Music signal too weak !\n");
	} else {
		unsigned long nHeadCut = iSampleStartG - lStartSampleGlobal;
		unsigned long nTailCut = (lStartSampleGlobal+lNumValidSamples-1)-iSampleStopG;
		lNumValidSamples -= nHeadCut+nTailCut;
		lStartSampleGlobal = iSampleStartG;
		if (nHeadCut > 0) {
			printf("Head cut, %8.4f(s) !\n", (double)nHeadCut/UNIFIED_SAMPLING_RATE);
		}
		if (nTailCut > 0) {
			printf("Tail cut, %8.4f(s) !\n", (double)nTailCut/UNIFIED_SAMPLING_RATE);
		}
	}

	if (lNumValidSamples < (MINIMUM_MUSIC_LENGTH_ALLOWED_IN_SECONDS*UNIFIED_SAMPLING_RATE)) {
		printf("Music signal too short, %8.4f(s), abort !\n", (double)lNumValidSamples/UNIFIED_SAMPLING_RATE);
		nReturn = -20; goto ExitHere;
	}
#endif
// head_tail_cut_002

	//������������ȷ���Ĺؼ������ݴ���ʱ�ļ�������д�롰�ؼ��������ļ���

	// input file of original samples
	fseek(m_pfProcessedSamps, sizeof(short)*lStartSampleGlobal, SEEK_SET);	// move to the right pos. for reading op.

	// �ļ�ͷ�� 3 ��Ԫ�أ�
	int& sr = nNumSkips;	// ���� "nNumSkips" ��
	sr = UNIFIED_SAMPLING_RATE;
	_write(fhw, &sr, sizeof(int));	// ����Ƶ�ʣ�������Ƿ������Ĳ���Ƶ�ʣ�
	_write(fhw, &lStartSampleGlobal, sizeof(unsigned long));	// index to the first valid sample
	_write(fhw, &lNumValidSamples, sizeof(unsigned long));	// number of valid samples

	lNeedToReadSize = lNumValidSamples;
	while (!feof(m_pfProcessedSamps) && lNeedToReadSize > 0) {
		nSizeRead = (lNeedToReadSize > nSizeInShorts)?nSizeInShorts:lNeedToReadSize;
		nSizeRead = fread(m_pdbData, sizeof(short), nSizeRead, m_pfProcessedSamps);
		if (nSizeRead <= 0) { // no data left or error
			break;
		}
		lNeedToReadSize -= nSizeRead;	// decrease
		_write(fhw, m_pdbData, nSizeRead*sizeof(short));
	}

ExitHere:
	if (pulStart && pulNumSamps) {
		*pulStart = lStartSampleGlobal;
		*pulNumSamps = lNumValidSamples;
	}
	return nReturn;
}

//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����һ�������ɵĲ���ֵ������������ǡ�Ŀ������ʡ��ˣ�������������Ӧ����������ͳ�ƣ�����
// "samps" �����ɵ�һ�������Ļ�����ַ
// "iSampStartGlobal" : ���������еĵ�һ��������ȫ�����
// "nNumSampsInBuffer" : ���������в�������
// 
void CMainSegment::DetectInWindow(short *samps, long iSampStartGlobal, int nNumSampsInBuffer)
{
	// �����ɵĲ������ݶ�д����ʱ�ļ���"m_pfProcessedSamps" ��ָ�ļ������Ա�ؼ���ȷ��֮��ֱ�ӴӴ��ļ�������
	//		���εĲ���ֵ����д�롰�ؼ��������ļ���
	// "m_pfProcessedSamps" ��ָ�ļ����ڴ��Ƿ�����������ϲ�����ѡ��һ��֮�����Ƶ�ź����ݡ�����ԭʼ��Ƶ����
	//		��Ӧ��ʱ����һ���ģ������������ǡ�Ŀ������ʡ��ˣ�������
	fwrite(samps, sizeof(short), nNumSampsInBuffer, m_pfProcessedSamps);
	m_lNumOriginalSamples += nNumSampsInBuffer;

	int iSampCur;
	for (iSampCur=0; iSampCur<nNumSampsInBuffer; iSampCur++) {
		if (_Xn_Max < (samps[iSampCur] > 0 ? samps[iSampCur]:-(long)samps[iSampCur]) ) {
			_Xn_Max = samps[iSampCur] > 0 ? samps[iSampCur]:-(long)samps[iSampCur];
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	long lOffset;
	int kkIndexLB, mmIndexLB;	// "LB" : Local Buffer

	iSampCur = iSampStartGlobal;
	while (iSampCur < iSampStartGlobal+nNumSampsInBuffer) {	// ���������ݻ�δ�����꣬����
		while (iSampCur < iSampStartGlobal+nNumSampsInBuffer	// �����еĲ���δ�����꣬������
			&& iSampCur < m_lStartSampIdxG_NextSkip	// ��ǰ skip ��δ����������
			) {
			m_dbValCurSkip += abs(samps[iSampCur-iSampStartGlobal]);	// �źŲ���ֵ�ľ���ֵ�ۼӣ�����
			iSampCur++;
		}
		if (iSampCur < m_lStartSampIdxG_NextSkip) {	// ���������ݴ������ˣ�����ǰ skip ��������ֻ���˳���
			break;
		}

// ��ǰ skip ��������������� skip ֵ���д���������

		m_dbValCurSkip /= m_usNumSampsInSkip;

		// ���Ե�ǰ skip Ϊͷ���ļ�ⴰ�ڵļ��ֵ��ŵ�λ�� "mmIndexLB" ��ָ������ "m_pdbData" �������е�λ�ã�
		lOffset = m_lCurSkipIdxG-m_lSkipIdxG_FirstValInBuffer;
		mmIndexLB = m_shFirstValIdxInBufferL+lOffset;
		if (mmIndexLB >= BufferSizeInDoubles) {// Խ���ˣ��ۻأ���ģ����ring buffer, ������
			mmIndexLB -= BufferSizeInDoubles; 
		}
		m_pdbData[mmIndexLB] = m_dbValCurSkip;	// 1st skip in the detection window
		for (kkIndexLB=m_shFirstValIdxInBufferL; kkIndexLB<m_shFirstValIdxInBufferL+lOffset; kkIndexLB++) {
			mmIndexLB = kkIndexLB;
			if (mmIndexLB >= BufferSizeInDoubles) {// Խ���ˣ��ۻأ���ģ����ring buffer, ������
				mmIndexLB -= BufferSizeInDoubles; 
			}
			// ��������ǰ skip �ļ�ⴰ�ڣ��� "mmIndexLB" ��ָ������Ҫ�ѵ�ǰ skip ��ֵ�ӵ�����ֵ��
			m_pdbData[mmIndexLB] += m_dbValCurSkip;
		}

		if	(m_lCurSkipIdxG-m_lSkipIdxG_FirstValInBuffer+1 == DetectWinSizeInSkips) {
		// ��ⴰ�ڣ�����ʼ skip ���Ϊ "m_lSkipIdxG_FirstValInBuffer"������ֵ���λ��
		// Ϊ "m_shFirstValIdxInBufferL"���ļ��ֵ������ˣ�����
		// Ҳ����˵�����ֵ�����еĵ�һ�����ֵ�Ѿ�����ȫ���ˣ�����ļ��ֵ��Ȼ������ȫ��Խ����Խ���Զ��
		// ע�⣺�������еĵ�һ�����ֵ������һ���ǻ����еĵ�һ����Ԫ�������еĵ�һ����Ԫ���±�Ϊ 0 ��������
			// ֻ�������ֵ������Ӧ�ļ�ⴰ����ʼ skip ��ţ�/�±꣩��������
			if (m_lSkipIdxG_MaxDetectVal == -1 || m_dbMaxDetectVal < m_pdbData[m_shFirstValIdxInBufferL]) {
				m_dbMaxDetectVal = m_pdbData[m_shFirstValIdxInBufferL];
				m_lSkipIdxG_MaxDetectVal = m_lSkipIdxG_FirstValInBuffer;
			}

			m_lSkipIdxG_FirstValInBuffer++;
			// ���ֵ�����е�λ�ã�������
			m_shFirstValIdxInBufferL++;
			if (m_shFirstValIdxInBufferL == BufferSizeInDoubles) {// Խ���ˣ����㣬������
				m_shFirstValIdxInBufferL = 0;
			}
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// prepare to process the next skip
		m_lCurSkipIdxG++;	// skip ���
		m_dbValCurSkip = 0.0;
		m_lStartSampIdxG_NextSkip += m_usNumSampsInSkip;	// �źŲ�����ȫ���±꣨/��ţ�

	}	// all new data has been processed
}


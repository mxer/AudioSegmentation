

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

int JoinSegments(SEG_INFO_T* pSegInfo, int& nNumSegs, int lenMin)
{
// ��ǣ�������ǣ�����ɾ����������ȥ�ĶΣ������Σ���������

	int nNumSegsDeleted = nNumSegs;	// �������
	int iSeg;
	int iSeg02 = -1;	// ǰһ�������ε��±�
	for (iSeg=0; iSeg<nNumSegs; iSeg++) {
		if (iSeg == 0) {// ��һ������Ҫ��ʱ������
			iSeg02 = iSeg;	// ������������ε��±�
			continue;
		}
//
		if (iSeg02 >= 0 &&	// ��ǰ�ε�����б����Σ�������
			pSegInfo[iSeg02].classNo == pSegInfo[iSeg].classNo) {// �ұ����ε�����뵱ǰ��һ�������뱣����
			pSegInfo[iSeg02].numFrames += pSegInfo[iSeg].numFrames;
			pSegInfo[iSeg].classNo = -(pSegInfo[iSeg].classNo+1);	// ���δ������ǣ������һ�Ա��� 0 ֵ��Ӱ�죡��
			continue;	// ����أ�����ֻ�е����ܲ�������ʱ���Żῼ�ǲ������ڣ�����
		}

// ����Ϊ��ͬ���֮��ĺϲ���������

	// ��+������Ȼ���㹻������������������
		if (pSegInfo[iSeg].numFrames >= lenMin) {// ֻҪ������Ҳ���Ǿ����Σ���������������
			// �����������������ֵ������Σ��ж������ն���
			for (int iSeg03=iSeg-1; iSeg03>=0; iSeg03--) {
				if (pSegInfo[iSeg03].classNo < 0) break;	// �ѱ���������գ��䵱Ȼ�������Σ���������
//
				if (pSegInfo[iSeg03].numFrames >= lenMin) break;	// һ�������������Σ����Σ������˳�ѭ����������
//
				// �������Σ�����֮��������
				pSegInfo[iSeg].numFrames += pSegInfo[iSeg03].numFrames;
				pSegInfo[iSeg03].classNo = -(pSegInfo[iSeg03].classNo+1);	// ��ɾ����ǣ������һ�Ա��� 0 ֵ��Ӱ�죡��
			}
//
			iSeg02 = iSeg;	// ���£���������εģ��±�
			continue;
		}

	// ��-������Ȼ�β���������������������

		// ������ڱ����Σ�������
		// �ɼ���һ���㹻���ı����ο�����������������ж��پ����ն��٣�������
		if (iSeg02 >= 0 && // ��ǰ�ε�����б����Σ�������
			pSegInfo[iSeg02].numFrames >= lenMin) {// �����ι����������մ˶Σ�������
			pSegInfo[iSeg02].numFrames += pSegInfo[iSeg].numFrames;	// ����ǰ�β���������������������εĽ���λ�ã�����
			pSegInfo[iSeg].classNo = -(pSegInfo[iSeg].classNo+1);	// ���δ������ǣ������һ�Ա��� 0 ֵ��Ӱ�죡��
			continue;	// ����أ�����ֻ�е����ܲ�������ʱ���Żῼ�ǲ������ڣ�����
		}

		// �˶��䲻�����������޷��������ڶΣ������ұ�����
		iSeg02 = iSeg;	// ������������ε��±�
	}

// ������������������ɾ���������˱�ǵĶΣ�������

	iSeg02 = 0;
	for (iSeg=0; iSeg<nNumSegs; iSeg++) {
		if (pSegInfo[iSeg].classNo < 0) continue;
		pSegInfo[iSeg02].numFrames = pSegInfo[iSeg].numFrames;
		pSegInfo[iSeg02].classNo = pSegInfo[iSeg].classNo;
		iSeg02++;
	}
	nNumSegs = iSeg02;	// �޸Ķ���

// ////////////////////////////////////////////////////////////////////////////////////////////

// �ϲ�ͬ��Σ�������

	int iClassNoPre;
	iSeg02 = 0;
	for (iSeg=0; iSeg<nNumSegs; iSeg++) {
		if (iSeg == 0 || pSegInfo[iSeg].classNo != iClassNoPre) {// �¿��Σ�������һ�Σ���������			
			pSegInfo[iSeg02].numFrames = pSegInfo[iSeg].numFrames;
			pSegInfo[iSeg02].classNo = pSegInfo[iSeg].classNo;
			iSeg02++;

			iClassNoPre = pSegInfo[iSeg].classNo;
		} else {// ͬ��Σ�����ǰ�Σ�ֻ��ǰ��֮�ҽ�
			pSegInfo[iSeg02-1].numFrames += pSegInfo[iSeg].numFrames;	// ע���±꣬"iSeg02-1" !!!
		}
	}
	nNumSegs = iSeg02;	// �޸Ķ���

// ////////////////////////////////

	nNumSegsDeleted -= nNumSegs;
	return nNumSegsDeleted;
}

#endif	// #ifndef __TRAINING_PHASE


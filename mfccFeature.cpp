

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

// MFCC ������������
// "m_pdbMFCC[...]" �����������У�ÿ֡һ������������
void CProcessingObj::MakeFeatureVectorMFCC()
{
	int ss, ii, mm;
	if (m_pMeanMFCC) {
		for (ii=0; ii<m_nNumFrames; ii++) {
			ss = NumDimsMfccM*ii;
			for (mm=0; mm<NumDimsMfccM; mm++) {
				m_pMeanMFCC[mm] += m_pdbMFCC[ss+mm];	// ��
				if (m_pStdMFCC)
					m_pStdMFCC[mm] += m_pdbMFCC[ss+mm]*m_pdbMFCC[ss+mm];	// ƽ����
			}
		}
		for (mm=0; mm<NumDimsMfccM; mm++) {
			m_pMeanMFCC[mm] /= m_nNumFrames;
			if (m_pStdMFCC) {
				m_pStdMFCC[mm] /= m_nNumFrames;
				m_pStdMFCC[mm] -= m_pMeanMFCC[mm]*m_pMeanMFCC[mm];	// ��ƽ���ľ�ֵ����ȥ����ֵ��ƽ����
				m_pStdMFCC[mm] = sqrt(m_pStdMFCC[mm]);	// �ٿ���
			}
		}
	}
// һ�ײ�֣�������
	if (m_pMeanDeriMFCC) {
		for (ii=1; ii<m_nNumFrames; ii++) {// form 1 on, ...
			ss = NumDimsMfccM*ii;
			for (mm=0; mm<NumDimsMfccM; mm++) {
				m_pMeanDeriMFCC[mm] += (m_pdbMFCC[ss+mm]-m_pdbMFCC[ss-NumDimsMfccM+mm]);	// ��ֺ�
				if (m_pStdDeriMFCC)
					m_pStdDeriMFCC[mm] += (m_pdbMFCC[ss+mm]-m_pdbMFCC[ss-NumDimsMfccM+mm])*
											(m_pdbMFCC[ss+mm]-m_pdbMFCC[ss-NumDimsMfccM+mm]);	// ���ƽ����
			}
		}
		for (mm=0; mm<NumDimsMfccM; mm++) {
			m_pMeanDeriMFCC[mm] /= m_nNumFrames-1;	// ������
			if (m_pStdDeriMFCC) {
				m_pStdDeriMFCC[mm] /= m_nNumFrames-1;	// ������
				m_pStdDeriMFCC[mm] -= m_pMeanDeriMFCC[mm]*m_pMeanDeriMFCC[mm];	// ����ֵ��ƽ��
				m_pStdDeriMFCC[mm] = sqrt(m_pStdDeriMFCC[mm]);
			}
		}
	}
}




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
#include <TCHAR.h>

#include "parameters.h"

#include "procobj.h"
#include "fft\fft.h"

int OpenFileRd(const char* psfn);

#ifndef __TRAINING_PHASE

// �ӿں�����1������������һ�Σ�����
int CProcessingObj::InitObj()
{
	 if (m_pWavInterface == NULL) { 
		m_pWavInterface = new CWavPreproc();
	 }
	 if (m_pWavInterface == NULL) {
		 return -1;
	 }

// �����ļ�����ʱ�ļ���������Ŀ����Ƶ��������

	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM);
	m_pWavInterface->CreateSampDataFile(m_szfn);

	return 0;
}

// �ӿں�����2������������ε��ã�����
int CProcessingObj::DataCome(const unsigned char* psamps, int numSamps)
{
	if (m_pWavInterface == NULL) return -1;
//
	m_pWavInterface->DataCome(psamps, numSamps);

	return 0;
}

// �ӿں�����3������������һ��
int CProcessingObj::OverHaHa(SEGMENT_VECTOR& segVector)
{
	if (m_pWavInterface == NULL) return -1;
//
	m_pWavInterface->DataCome(NULL, 0);	// ֪ͨ����������

// ��Ŀ���źŲ��������ļ�����ʱ�ļ�����Ϊ�����ݣ�������

	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM);
	int fhr = OpenFileRd(m_szfn);
	if (SegmentAudio(fhr, segVector) != 0) {
	}
	_close(fhr);

// ɾ���ļ�����ʱ�ļ�����������
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM);
	remove(m_szfn);

	return 0;
}

#endif	// #ifndef __TRAINING_PHASE

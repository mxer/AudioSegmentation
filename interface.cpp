

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

#include "parameters.h"
#include "interface.h"

#ifndef BUFFER_SIZE_IN_SAMPLES_DS
#define BUFFER_SIZE_IN_SAMPLES_DS	8192
#endif

int OpenFileWr(const char* psfn);

void CWavPreproc::CreateSampDataFile(const char* psfn)
{
	// ������ʱ�ļ�
	m_fhw = OpenFileWr(psfn);
	// ������ļ�ͷ�ϱ����ռ䣬Ϊ���桰Ŀ������ʡ��͡�Ŀ�����������2 ������
	_lseek(m_fhw, sizeof(int)+sizeof(unsigned long), SEEK_SET);
//
	// total number of down-sampling samples made 
	m_nNumNewlyMadeSampsTotal = 0;
	// ��ԭ�������桱�е�һ��λ�õĲ������ȫ���±�
	m_iOriginalSampStartG = 0;
	// �������ɵĵ�һ��Ƿ�����㣨����Ƿ�������桱�е�һ�������㣩��ȫ���±�
	m_iTargetSampStartG = 0;

	short* ptmp = (short*)realloc(m_NewlyMadeSampsBuff, BUFFER_SIZE_IN_SAMPLES_DS*sizeof(short));
	if (ptmp) {
		m_NewlyMadeSampsBuff = ptmp;
	}
}

// "nNumOrigSamps" ԭʼ����������һ��ԭʼ��������������һ������ʱ�����������������
void CWavPreproc::DataCome(const unsigned char* pOrigSamps, int nNumOrigSamps)
{
// ע�⣺Ƿ�����źź�ԭ�����źŶ�Ӧ��ʱ����һ���ģ�����

	if (pOrigSamps == NULL || nNumOrigSamps <= 0) {// ��ʾ�����Ѹ��꣬������
		if (m_NewlyMadeSampsBuff) {
			free(m_NewlyMadeSampsBuff);
			m_NewlyMadeSampsBuff = NULL;
		}
//
		int sr = UNIFIED_SAMPLING_RATE;
		_lseek(m_fhw, 0, SEEK_SET);	// �ļ�ͷ
		_write(m_fhw, &sr, sizeof(int));	// Ŀ�������
		_write(m_fhw, &m_nNumNewlyMadeSampsTotal, sizeof(unsigned long));	// Ŀ���������
//
		_close(m_fhw);
		m_fhw = -1;
//
		free(m_NewlyMadeSampsBuff);
		m_NewlyMadeSampsBuff = NULL;
//
		printf("Length of clip in seconds : %f\n", (double)m_nNumNewlyMadeSampsTotal/UNIFIED_SAMPLING_RATE);

		return;
	}

// ����������һ��ԭʼ�������ݣ�������

	// ÿ����һ����ԭ�������ݣ�������һ��Ƿ������
	int nNumNewlyMadeSamps = 0;	// ���������ӱ�������ԭʼ�ź����ݣ����ɵ�Ƿ���������

	if (m_iTargetSampStartG == 0) {
	// ���������Ƿ��������һ��ԭʼ��������Ҫֱ�ӿ����ġ�Ҳ����˵��Ƿ�����źŵĵ�һ�����������ԭ�źŵĵ�һ��
	// �����㣡
		assert(nNumNewlyMadeSamps == 0);
		// ��һ������ֵ�հ�

#ifdef __COMBINE_L_R_CHANNELS	// clrc_001
// ȡ����������ƽ��
		if (num_of_channels_ > 1) {
			long lval;	// �� "long" ��֤������̲����
			if (m_header.bit_p_sample == 16) {
				lval = *((short *)pOrigSamps);
				lval += *(((short *)pOrigSamps)+1);
			} else if (m_header.bit_p_sample == 8) {
				lval = *((char *)pOrigSamps);
				lval += *(((char *)pOrigSamps)+1);
				lval *= 256;
			}
			lval /= 2;
			m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] = lval;
		} else {// ֻ��һ������ʱ��������
			if (m_header.bit_p_sample == 16) {
				m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((short *)pOrigSamps);
			} else if (m_header.bit_p_sample == 8) {
				m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((char *)pOrigSamps);
				m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] *= 256;
			}
		}
#else
// ֻȡ��һ������
		if (bit_p_sample_ == 16) {
			m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((short *)pOrigSamps);	// ��һ������ֵ�հ�
		} else if (bit_p_sample_ == 8) {
			m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((char *)pOrigSamps);	// ��һ������ֵ�հ�
			m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] *= 256;
		}
#endif	// clrc_001
		nNumNewlyMadeSamps++;
	}

	//���������ɶ�Ӧ���²�����������
	while (1) {
	// �����ǽ�Ƿ�������Ӧ��ĳ��ԭ�����㣨ֻ��������㣩�����������ڵ�ԭ�����㣨���������������㣩֮�䣡����

		long iTargetSampG, iOriginalSampG;
		// ����Ƿ�����㣨Ŀ������㣩�±꣬������
		iTargetSampG = m_iTargetSampStartG+nNumNewlyMadeSamps;
		// �������Ӧ��ԭ�������±�
		iOriginalSampG = (double)iTargetSampG*sample_rate_/UNIFIED_SAMPLING_RATE+0.5;
		if (iOriginalSampG > m_iOriginalSampStartG+nNumOrigSamps-1) {
		// ��δ������Ŀ��ԭ�����㡱��������
			m_iOriginalSampStartG += nNumOrigSamps;
			break;
		}
		// Ŀ��ԭ�������ڻ����е�λ��
		const unsigned char *puchar = pOrigSamps+(byte_p_sample_)*(iOriginalSampG-m_iOriginalSampStartG);
#ifdef __COMBINE_L_R_CHANNELS	// clrc_002
// ȡ����������ƽ��
		if (num_of_channels_ > 1) {
			long lval;
			if (m_header.bit_p_sample == 16) {// 16 λ����
				lval = *((short *)puchar);	// �������ĵ�һ������
				lval += *(((short *)puchar)+1);	// �������ĵڶ�������
			} else if (m_header.bit_p_sample == 8) {// 8 λ����
				lval = *((char *)(puchar));	// �������ĵ�һ������
				lval += *(((char *)puchar)+1);	// �������ĵڶ�������
				lval *= 256;
			}
			lval /= 2;
			m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] = lval;
		} else {// ֻ��һ������ʱ��������
			if (m_header.bit_p_sample == 16) {
				m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((short *)puchar);
			} else if (m_header.bit_p_sample == 8) {
				m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((char *)puchar);
				m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] *= 256;
			}
		}
#else
// ֻȡ��һ������
		if (bit_p_sample_ == 16) {
			m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((short *)puchar);
		} else if (bit_p_sample_ == 8) {
			m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((char *)puchar);
			m_NewlyMadeSampsBuff[nNumNewlyMadeSamps] *= 256;
		}
#endif	// clrc_002
		nNumNewlyMadeSamps++;	// ����Ƿ������������
			
	}	// end of "while (1) {"

	//�����������������ɵĲ���������Ƿ�������������ϲ���Ĳ������ݣ�д������ļ�
	if (_write(m_fhw, m_NewlyMadeSampsBuff, nNumNewlyMadeSamps*sizeof(short)) != nNumNewlyMadeSamps*sizeof(short)) {
		printf("Error writing temp sample data file !\n");
	}

	//���ģ�׼���������ļ�������һ��ԭ�������ݣ�������
	m_iTargetSampStartG += nNumNewlyMadeSamps;
	m_nNumNewlyMadeSampsTotal += nNumNewlyMadeSamps;

/*
	if (pOrigSamps)
		free(pOrigSamps);
*/
}

// ���캯��
CWavPreproc::CWavPreproc()
{
	m_fhw = -1;
	m_NewlyMadeSampsBuff = NULL;	// ����ģ�����
//
	assert(bit_p_sample_ == 16 || bit_p_sample_ == 8);
	// "m_header.sample_rate" : ԭʼ����Ƶ��
	// ԭʼ�����ʱ�����ڻ���� "UNIFIED_SAMPLING_RATE"���˼�Ŀ������ʣ�������
	if (sample_rate_ < UNIFIED_SAMPLING_RATE) {
		printf("Original sampling rate(%d) less than target !\n", sample_rate_);
	}
}

CWavPreproc::~CWavPreproc()
{
	if (m_NewlyMadeSampsBuff)
		free(m_NewlyMadeSampsBuff);
}


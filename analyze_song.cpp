

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
	if (SegmentAudio(fhandle, segVector) < 0) {
	} else {
		JoinSameClassSegs(segVector);
		SaveSegInfoToFile(segVector);
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

	int maxAbs;	// �źŲ�������ֵ�����ֵ
	int nReturn = m_pfWaveform->MakeTargetSamplesData(fhandle, maxAbs);	// ����Ŀ���źŲ�������
	_close(fhandle);
	m_pfWaveform->CloseWaveFile();
//
	if (nReturn < 0) {// ����Ŀ���źŲ�������ʱ���ִ���ɾ����ʱ�ļ�
		sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM);
		remove(m_szfn);
		return ERROR_NO_TMP_SAMPLE_DATA_FILE;
	}

#ifdef __NORMALIZE_AUDIO_SAMPLES
	if (NormalizeSamples(maxAbs) < 0) {
	}
#endif

	return 0;
}

#ifdef __NORMALIZE_AUDIO_SAMPLES

#define BUFFER_SIZE_IN_SAMPS	16384	// 32K
//#define MAX_ABS_SMPLE_VALUE		32767.0

int CProcessingObj::NormalizeSamples(int maxAbs)
{
	if (maxAbs <= 0) return -1;

	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM);
	int fhr = OpenFileRd(m_szfn);
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM02);
	int fhw = OpenFileWr(m_szfn);

	short *pBufShorts = (short*)malloc(sizeof(double)*BUFFER_SIZE_IN_SAMPS);	// ע�⣺�ռ��� double ������
	if (pBufShorts == NULL) {
		_close(fhr);
		_close(fhw);
	}
	double *pBufDobles = (double*)(pBufShorts);	// �ص�

	int sr;
	unsigned long istart, num_samples;
	if (_read(fhr, &sr, sizeof(int)) != sizeof(int)) {
	}
	if (_read(fhr, &istart, sizeof(unsigned long)) != sizeof(unsigned long)) {
	}
	if (_read(fhr, &num_samples, sizeof(unsigned long)) != sizeof(unsigned long)) {
	}
//
	if (_write(fhw, &sr, sizeof(int)) != sizeof(int)) {
	}
	if (_write(fhw, &istart, sizeof(unsigned long)) != sizeof(unsigned long)) {
	}
	if (_write(fhw, &num_samples, sizeof(unsigned long)) != sizeof(unsigned long)) {
	}
	int& nReturn = sr;
	nReturn = 0;
	while (1) {
		int nNumBytes = _read(fhr, pBufShorts, BUFFER_SIZE_IN_SAMPS*sizeof(short));
		if (nNumBytes <= 0)
			break;
		nNumBytes /= sizeof(short);
		for (int ii=nNumBytes-1; ii>=0; ii--) {// ע�⴦��˳��
			pBufDobles[ii] = (double)pBufShorts[ii]/maxAbs;
		}
		if (_write(fhw, pBufDobles, nNumBytes*sizeof(double)) != nNumBytes*sizeof(double)) {
			printf("Error writing file, NormalizeSamples(int maxAbs) !\n");
			nReturn = -1;
			break;
		}
	}
	free(pBufShorts);
	_close(fhw);
	_close(fhr);
	if (nReturn < 0) {
		return -5;
	}

/*	// Rename file
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM);
	remove(m_szfn);
	char szfn_tmp[260];
	sprintf(szfn_tmp, "%s%s", m_psOutPath, FN_TEMP_WAVEFORM02);
	rename(szfn_tmp, m_szfn);*/

	return 0;
}

#endif	// #ifdef __NORMALIZE_AUDIO_SAMPLES

#ifndef __TRAINING_PHASE

#endif	// #ifndef __TRAINING_PHASE


#ifdef __123_ABC
int CProcessingObj::ProcessAllFilesInDirHelper(char *psPath)
{
	_finddata_t fileinfo;
	int ipos = strlen(psPath);

	strcat(psPath, "*.*");
	long hFile = _findfirst(psPath, &fileinfo);	
	if ( hFile == -1 )  
		return -1;

	do {		
		if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
			continue;

		psPath[ipos] = '\0';
		strcat(psPath, fileinfo.name);	// "fileinfo.name" may be file name or directory name
		if ( (fileinfo.attrib & _A_SUBDIR) ) {
		// this is a directory, ...

			// if the corresponding output sub dir doesn't exist, create it
			// Ϊ�˱�֤�����Ŀ¼�µ�Ŀ¼�ṹ�������Ŀ¼�µ�Ŀ¼�ṹһ�£���Ϊ����һ�������ļ���Ӧһ������ļ���
			// Ҫ���첻����������ļ���������������������
			sprintf(m_szfn, "%s%s", m_psOutPath, psPath+strlen(m_psInPath));
			if (_access(m_szfn, 0) == -1) {
				_mkdir(m_szfn);
			}

			strcat(psPath, "\\");
			// �����Ŀ¼���ݹ���ã�������
			ProcessAllFilesInDirHelper(psPath);
		} else {
		// this is a file, process it, ... 
/*			if (AnalyzeWaveSong(psPath) != 0) {
				printf("Error processing file %s !\n", psPath);
			}*/
		}
	} while (_findnext(hFile, &fileinfo) == 0);

	_findclose(hFile);

	return 0;	// successful
}
#endif


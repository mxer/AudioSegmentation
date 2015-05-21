

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

#include <conio.h>	// for "_getch()"

#include "parameters.h"
#include "procobj.h"
#include "fft\fft.h"

#ifdef __TRAINING_PHASE

#include "svmInterface.h"

int WriteVectorToTrainingDataFile(const double *pdbvector, int nNumDims, int nClassNo, FILE *pfw_tr);

//bool ppsvmTrain(const char *psfn_trainingData, int num_dims);
//void ppsvmDeleteTrainingDataFiles();

#include <TCHAR.H>

// ѵ����������������
// ���룺ѵ�������б��ļ�
// ��ѵ�������б��ļ����������������������
// "psfn_examps" ѵ����������Ƶ�ļ����б��ļ���������·������
int CProcessingObj::TrainClassifier(const char* psfn_examps)
{
	m_nNumFilesAll = 0;

	m_nNumInvalidExamples = 0;	// �źŲ�����Ҫ�����������
//
	// ����ѵ�����������ļ�
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TRAINING_DATA_ORGINAL);
	m_pfTrainingDataTxt = fopen(m_szfn, "wt");	// �����ļ�
//
	// ��ѵ����������Ƶ�ļ���������·���� - ���š��б��ļ�������ÿ�б�ʾһ��ѵ������
	sprintf(m_szfn, "%s%s", m_psInPath, psfn_examps);
	FILE* pfr_t = fopen(m_szfn, "rt");	// ��
	TCHAR* szline = Malloc(TCHAR, 512);
	TCHAR* psPos;
	while (1) {
		if (_fgetts(szline, 512, pfr_t) == NULL) {	break; }
		if (szline[0] == _T('\0')) { break; }
		// �һس����з����ضϣ�������
		psPos = szline;
		while (*psPos != _T('\0')) {
			if (*psPos == _T('\r') || *psPos == _T('\n')) { *psPos = _T('\0'); break; }
			psPos = _tcsinc(psPos);
		}
		// Լ���ļ���������֮���� '\t' ����
		psPos = _tcschr(szline, _T('\t'));	// 1st occurrence of '\t'
		*psPos = _T('\0');
		psPos = _tcsinc(psPos);

		m_nNumFilesAll++;
		printf("\n\nTraining example No.%d\n", m_nNumFilesAll);
		if (ProcessExample(szline, atoi(psPos)) != 0) {

		}
	}
	fclose(pfr_t);
	free(szline);
//
	fclose(m_pfTrainingDataTxt);
	m_pfTrainingDataTxt = NULL;

	printf("\n��Ч����������%d\n", m_nNumInvalidExamples);
//	fprintf(, "\n�� %d �����������к� %d ����Ч������\n", m_nNumFilesAll, m_nNumInvalidExamples);

#ifdef __GENERATE_WEKA_TRAINING_FILE
	return 0;
#endif	// __GENERATE_WEKA_TRAINING_FILE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
	CSvmInterface svm(m_psInPath, m_psOutPath);

// ѵ����������ѵ�������󽫷�����ģ�����ݴ����ļ�����������
	printf("\nBegin to train classifier, ...\n");
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TRAINING_DATA_ORGINAL);
	svm.ppsvmTrain(m_szfn, m_nNumDims);
//
	// ɾ��ѵ�����������ļ�
	sprintf(m_szfn, "%s%s", m_psOutPath, FN_TRAINING_DATA_ORGINAL);
	remove(m_szfn);

	return 0;
}

#define NUM_SKIPS_LOCAL	10	// �ŵ���������֡������֡���ص���

// "fhr" ѵ�������ź������ļ���������
// ��һ����Ƶ�ļ������γɶ��ѵ������
int CProcessingObj::ProcessTrainingExample(int fhr)
{
	int nBufSizeShorts = NumSamplesInFrameSkipM*(NUM_SKIPS_LOCAL-1)+NumSamplesInFrameM;
	short* pBuffer = Malloc(short, nBufSizeShorts);
	if (pBuffer == NULL) {
		return -1;
	}
#ifdef _DEBUG
	int sr;
	unsigned long istart, num_samples;
	if (_read(fhr, &sr, sizeof(int)) != sizeof(int)) {
	}
	if (_read(fhr, &istart, sizeof(unsigned long)) != sizeof(unsigned long)) {
	}
	if (_read(fhr, &num_samples, sizeof(unsigned long)) != sizeof(unsigned long)) {
	}
#else
	_lseek(fhr, sizeof(int)+sizeof(unsigned long)+sizeof(unsigned long), SEEK_SET);
#endif

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	m_nNumFrames = 0;	// ��ǰ�������ĵ�ǰ��Ԫ��������ȡ��֡��

	short* pCurFrame;
	int nShorts = 0;	// a must !
	int nBytesRead;
	while (1) {
		nBytesRead = _read(fhr, pBuffer+nShorts, (nBufSizeShorts-nShorts)*sizeof(short));
		if (nBytesRead <= 0) {// ���������ļ��е����ݶ����ˣ����˳�
			break;
		}
//
		nShorts += nBytesRead/sizeof(short);	// �����в�����short������
		pCurFrame = pBuffer;
		while (nShorts >= NumSamplesInFrameM) {// ����������һ֡��������
			ProcSingleFrame(pCurFrame);
			m_nNumFrames++;
//
			if (m_nNumFrames == NumFrameSkipsInUnitM) {// ��һ����Ԫ�ˣ�������
				printf("Number of frames : %d\n", m_nNumFrames);
				// �˵�Ԫ�γ�һ��ѵ������
				MakeAndWriteTrainingExample(true);
			}

			pCurFrame += NumSamplesInFrameSkipM;	// ֡��һ��
			nShorts -= NumSamplesInFrameSkipM;
		}
		if ((nBytesRead % sizeof(short)) != 0) {// �ļ��д�����
			printf("Invalid sample data !\n");
			break;
		}
//
		// ʣ��ġ�����һ֡�Ĳ��������Ƶ�������
		memmove(pBuffer, pCurFrame, sizeof(short)*nShorts);
	}

	free(pBuffer);

	printf("Number of frames : %d\n", m_nNumFrames);
	if (m_nNumFrames < MINIMUM_NUM_FRAMES_IN_UNIT) {// ѵ���������������һ����Ԫ��̫�̣�����
		printf("Example too short to be used !");
		return 0;
	}
//
	// �������һ����Ԫ�����أ�������
	return MakeAndWriteTrainingExample(false);
}

int CProcessingObj::MakeAndWriteTrainingExample(bool bMove)
{
	int nReturn = 0;
	// ������������������������
	if (MakeFeatureVector() == 0) {
		// ������������������������д��ѵ�����������ļ�
		if (WriteVectorToTrainingDataFile(m_pvector, m_nNumDims, m_iClassNo) != 0) {
			printf("Error writing training example to file !\n");
			nReturn = -1;
		}
	} else {
		m_nNumInvalidExamples++;
		printf("Error generating training example feature vector !\n");
		nReturn = -2;
	}

	if (bMove) {// ��Ԫ֮����ص������Ƶ�������
		MoveUnitOverlap();
	}

	return nReturn;
}

// ��ѵ����������д�� SVM ������ѵ����������ĸ�ʽ����дһ�����������ݣ���������
// "nClassNo" ������������ţ�from 1 on, ...
// "nNumDims" ��������������ά��
// 
int CProcessingObj::WriteVectorToTrainingDataFile(const double *pdbvector, int nNumDims, int nClassNo)
{
#ifdef __GENERATE_WEKA_TRAINING_FILE
	for (int idim=0; idim<nNumDims; idim++) {
		if (!_finite(pdbvector[idim])) {
			printf("dimension %d invalid value !\n", idim+1);
			_getch();
		}
		fprintf(m_pfTrainingDataTxt, "%g ", pdbvector[idim]);
	}
	fprintf(m_pfTrainingDataTxt, "%d\n", nClassNo);	// ���ѵ������������ţ�1����

#else
	fprintf(m_pfTrainingDataTxt, "%d ", nClassNo);	// ���ѵ������������ţ�1����
	// ���������
	int idim;
	for (idim=0; idim<nNumDims-1; idim++) {// !!! subtract one !!!
		if (!_finite(pdbvector[idim])) {
			printf("dimension %d invalid value !\n", idim+1);
			_getch();
		}
		fprintf(m_pfTrainingDataTxt, "%d:%g ", idim+1, pdbvector[idim]);
	}
	if (!_finite(pdbvector[idim])) {
		printf("dimension %d invalid value !\n", idim+1);
		_getch();
	}
	fprintf(m_pfTrainingDataTxt, "%d:%g\n", idim+1, pdbvector[idim]);

#endif

	return 0;
}


#endif	// __TRAINING_PHASE


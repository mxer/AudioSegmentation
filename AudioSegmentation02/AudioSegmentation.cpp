// AudioSegmentation02.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <io.h>	// for "_access(...)"
//#include <errno.h>
//#include <direct.h>

//#include <conio.h>	// for "_getch()"

//#include <ctype.h>

//#include <time.h>
//#include <sys/types.h>
//#include <sys/timeb.h>
//#include <math.h> 
#include <assert.h>

#include "MyAudioSegmentationDll.h"

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef __TRAINING_PHASE
// ������ѵ�����̣�3 ���������������������ļ�Ŀ¼������ļ�Ŀ¼
	int nNumParameters = 3;
#else
// ��Ƶ���ࣨ����ֶΣ����̣�4 ���������������������ļ�Ŀ¼������ļ�Ŀ¼�����ֶ���Ƶ�ļ�������ȫ·����
	int nNumParameters = 4;
#endif	// __TRAINING_PHASE
	if (argc != nNumParameters) {
		printf("%d command line parameters, less than %d !\n", argc, nNumParameters);
		return -1;
	}
	if (_access(argv[1], 0) != 0) {
		printf("Path %s does not exist !\n", argv[1]);
		return -1;
	}
	if (_access(argv[2], 0) != 0) {
		printf("Path %s does not exist !\n", argv[2]);
		return -1;
	}
#ifndef __TRAINING_PHASE
	if (_access(argv[3], 0) != 0) {// ���ֶε���Ƶ�ļ�������·����
		printf("File %s does not exist !\n", argv[3]);
		return -1;
	}
#endif	// __TRAINING_PHASE

	InitDll(argv[1], argv[2]);
#ifdef __TRAINING_PHASE
	// "FN_TRAINING_EXAMPLES_LIST" ѵ����������Ƶ�ļ����б��ļ��������ļ���������·������
	TrainClassifier(FN_TRAINING_EXAMPLES_LIST);
#else
	// ��ŷֶ���Ϣ������
	SEGMENT_VECTOR segVector;
//
	// "argv[3]" ���ֶε���Ƶ�ļ���������·����
	SegmentAudioPiece(argv[3], segVector);

	// ����������

	segVector.clear();

#endif	// __TRAINING_PHASE
	FreeDllMemory();	// �ͷ��ڴ�

	return 0;
}




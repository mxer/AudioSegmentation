// MyDll.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include <assert.h>

#include "MyAudioSegmentationDll.h"

#include "procobj.h"

const char *g_psOutRootPath = 0;
const char *g_psInRootPath = 0;
static FILE *g_pfout = NULL;

static CProcessingObj* g_pMainObj = NULL;

#ifdef __TRAINING_PHASE

#include <malloc.h>

#else	// __TRAINING_PHASE

#include <map>
using namespace std;
// ���±����������ձ�
typedef map<int, string, less<int>> ClassNo_LABEL_MAP;
typedef pair <int, string> IntStringPairT;
// ���±����������ձ�
static ClassNo_LABEL_MAP g_ClassNoLabelMap;
FILE* g_pfwInfo = NULL;
#endif	// __TRAINING_PHASE

// ֻ�ܱ���һ�Σ��� DLL װ��ʱ
MY_AUDIO_SEGMENTATION_DLL_API int InitDll(const char* argv1, const char* argv2)
{
	if (g_pMainObj != NULL) {
		printf("Doing nothing, g_pMainObj already exists!\n");
		return 1;
	}

	char* psfn;
#ifndef __TRAINING_PHASE
	// ����±����������ձ�������
	g_ClassNoLabelMap.clear();
	g_ClassNoLabelMap.insert(IntStringPairT(0, "������"));
	g_ClassNoLabelMap.insert(IntStringPairT(1, "���ֶ�"));
	g_ClassNoLabelMap.insert(IntStringPairT(2, "������"));
	g_ClassNoLabelMap.insert(IntStringPairT(3, "��϶�"));
	g_ClassNoLabelMap.insert(IntStringPairT(4, "������"));

	// �����ֶ���Ϣ�ļ���������·����
	psfn = Malloc(char, 260);
	sprintf(psfn, "%s%s", argv2, "seg_info.txt");
	g_pfwInfo = fopen(psfn, "wt");
	assert(g_pfwInfo);
	free(psfn);
	psfn = NULL;
//
#endif	// __TRAINING_PHASE

	g_psOutRootPath = _strdup(argv2);	// ����
	g_psInRootPath = _strdup(argv1);	// ����

	// ���������Ϣ�ļ�
	psfn = Malloc(char, 256);
	sprintf(psfn, "%s%s", argv2, "informtion.txt");
	g_pfout = fopen(psfn, "wt");
	assert(g_pfout);
	free(psfn);
	psfn = NULL;
//

	g_pMainObj = new CProcessingObj();
	if (g_pMainObj == NULL) {
		printf("Can't create CProcessingObj object !\n");
		return -2;
	}

	g_pMainObj->SetPath(argv1, argv2);	// ע�⣺������������·����ĩβ������ "/" ��
	return 0;
}

#ifdef __TRAINING_PHASE
MY_AUDIO_SEGMENTATION_DLL_API int TrainClassifier(const char* psfn_examps)
{
	if (g_pMainObj == NULL) {
		printf("g_pMainObj == NULL !\n");
		return -1;
	}

	// "psfn_examps" ѵ����������Ƶ�ļ����б��ļ���������·������
	g_pMainObj->TrainClassifier(psfn_examps);
	return 0;
}
#else
MY_AUDIO_SEGMENTATION_DLL_API int SegmentAudioPiece(const char* psfn, SEGMENT_VECTOR& segVector)
{
	if (g_pMainObj == NULL) {
		printf("g_pMainObj == NULL !\n");
		return -1;
	}

	if (g_pfwInfo) {
		fprintf(g_pfwInfo, "\n+++\nAudio File : %s\n\n", psfn);
	}
	g_pMainObj->ProcessExample(psfn, segVector);	// "psfn" ���ֶε���Ƶ�ļ���������·����
	return 0;
}
MY_AUDIO_SEGMENTATION_DLL_API const char* ClassLabel(int nClassNo)
{
	ClassNo_LABEL_MAP::const_iterator iter_x = g_ClassNoLabelMap.find(nClassNo);
	if (iter_x == g_ClassNoLabelMap.end()) return NULL;
	return iter_x->second.c_str();
}
#endif	// __TRAINING_PHASE

// ֻ�ܱ���һ�Σ��� DLL ж��ʱ
MY_AUDIO_SEGMENTATION_DLL_API void FreeDllMemory()
{
	if (g_pMainObj) {
		delete g_pMainObj;
		g_pMainObj = NULL;
	}

#ifndef __TRAINING_PHASE
	g_ClassNoLabelMap.clear();
//
	if (g_pfwInfo) {
		fclose(g_pfwInfo);
		g_pfwInfo = NULL;
	}
#endif	// __TRAINING_PHASE

	if (g_pfout) {
		fclose(g_pfout);
		g_pfout = NULL;
	}

// ///////////////////////////////////

	if (g_psOutRootPath) {
		free((void *)g_psOutRootPath);
		g_psOutRootPath = NULL;
	}
	if (g_psInRootPath) {
		free((void *)g_psInRootPath);
		g_psInRootPath = NULL;
	}
}



/*
// ���ǵ���������һ��ʾ��
MY_AUDIO_SEGMENTATION_DLL_API int nMyDll=0;

// ���ǵ���������һ��ʾ����
MY_AUDIO_SEGMENTATION_DLL_API int fnMyDll(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� MyDll.h
CMyDll::CMyDll()
{
	return;
}
*/

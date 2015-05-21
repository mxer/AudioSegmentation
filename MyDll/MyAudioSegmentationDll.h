
#ifndef __MY_AUDIO_SEGMENTATION_DLL_HHH__
#define __MY_AUDIO_SEGMENTATION_DLL_HHH__

// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵ĺ�ı�׼������
// �� DLL �е������ļ��������������϶���� MY_AUDIO_SEGMENTATION_DLL_EXPORTS ���ű���ġ�
// ��ʹ�ô� DLL ���κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// MY_AUDIO_SEGMENTATION_DLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef MY_AUDIO_SEGMENTATION_DLL_EXPORTS
#define MY_AUDIO_SEGMENTATION_DLL_API __declspec(dllexport)
#else
#define MY_AUDIO_SEGMENTATION_DLL_API __declspec(dllimport)
#endif

#include "mydefs.h"	// ȫ�ֺ궨�� - ��������

#ifndef __TRAINING_PHASE
#include "myVector.h"
#endif	// __TRAINING_PHASE

// "argv1" �����ļ���·����������ģ���ļ����ڴˣ��������ԡ�\����β
// "argv2" ����ļ���·������ʱ�ļ����ڴˣ��������ԡ�\����β
MY_AUDIO_SEGMENTATION_DLL_API int InitDll(const char* argv1, const char* argv2);
#ifdef __TRAINING_PHASE
// "psfn_examps" ������ѵ����������Ƶ�ļ����б��ļ��������ļ���������·������
MY_AUDIO_SEGMENTATION_DLL_API int TrainClassifier(const char* psfn_examps);
#else
// "psfn" ���ֶε���Ƶ�ļ���������·����
MY_AUDIO_SEGMENTATION_DLL_API int SegmentAudioPiece(const char* psfn, SEGMENT_VECTOR& segVector);
// "nClassNo" ����ţ�0 ����
MY_AUDIO_SEGMENTATION_DLL_API const char* ClassLabel(int nClassNo);
#endif	// __TRAINING_PHASE
MY_AUDIO_SEGMENTATION_DLL_API void FreeDllMemory();

#endif	// __MY_AUDIO_SEGMENTATION_DLL_HHH__

//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

/*
// �����Ǵ� MyDll.dll ������
class MY_AUDIO_SEGMENTATION_DLL_API CMyDll {
public:
	CMyDll(void);
	// TODO: �ڴ�������ķ�����
};

extern MY_AUDIO_SEGMENTATION_DLL_API int nMyDll;

MY_AUDIO_SEGMENTATION_DLL_API int fnMyDll(void);
*/

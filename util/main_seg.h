
#ifndef	__CMAIN_SEG_Q__
#define __CMAIN_SEG_Q__

#include <stdio.h>	// file

// Ϊ�˼����Ƶ�ź��еĹؼ��Σ�ƽ���������ĶΣ�

class CMainSegment {
private:
	// ��ⴰ�������Ĵ�С���̶�Ϊ 30 ����ʱ����Ӧ���źŲ����������롰Ŀ������ʡ��йأ�
	unsigned short m_usNumSampsInSkip;

	unsigned short _Xn_Max;	// ��ǰ�������ź�ֵ����ֵ���ֵ

private:
	// ���ļ����ڴ��Ƿ�����������ϲ�����ѡ��һ��֮�����Ƶ�ź����ݡ���ԭʼ��Ƶ���ݶ�Ӧ��ʱ��һ��������
	FILE *m_pfProcessedSamps;

	// buffer to hold signal sample data
	double *m_pdbData;

	// "LB" : Local Buffer; "GB" : Global Buffer

	// �����ļ������� skips ���ɣ�ͷβ��ӣ����ص�����skip ��Ŵ� 0 ��ʼ
	// ÿ�� skip ����Ӧһ����ⴰ�ڣ��Ը� skip ��ͷ�ļ�ⴰ�ڣ��Ը� skip �������Ϊ��ʶ����ÿ����ⴰ�ڶ�Ӧһ�����ֵ
	// ������ "m_pdbData" �еġ���һ�����ֵ����Ӧ�� skip ��ţ���Ŵ� 0 ��ʼ����ָ�Ը� skip ��ʼ�ļ�ⴰ�ڵļ��ֵ����
	long m_lSkipIdxG_FirstValInBuffer;
	// ������ "m_pdbData" �еġ���һ�����ֵ�����±꣨�� 0 ��ʼ������ڻ���������ʼλ�ã�
	short m_shFirstValIdxInBufferL;

	long m_lCurSkipIdxG;
	double	m_dbValCurSkip;

	// ��һ�� skip ����ʼ�������±�
	unsigned long m_lStartSampIdxG_NextSkip;

	// ԭʼ��������������Ƿ����
	unsigned long m_lNumOriginalSamples;

	// ��¼��Ѷε�λ�ã��� skip ��ű�ʾ���ͼ��ֵ
	long m_lSkipIdxG_MaxDetectVal;
	double m_dbMaxDetectVal;

private:
	unsigned short m_usMaxMainSegSizeInSkips;

public:
	void SetMaxMainSegSize(unsigned short usMaxMainSegSize) { m_usMaxMainSegSizeInSkips = usMaxMainSegSize; }

public:
	// ÿ���һ��Ƿ�������ݺ󣬾͵��ô˺���
	void DetectInWindow(short *samps, long iLowSampStart, int nNumLowSamps);

//	void StartDetection();
//	void DetectionEnd();
	int WriteMainSegToFile(int fhw, unsigned long *pulStart, unsigned long *pulNumSamps);

public:
	CMainSegment();
	virtual ~CMainSegment();

	bool IsCreatedOK() { return (m_pdbData != 0 && m_pfProcessedSamps != 0); }
};

#endif	// __CMAIN_SEG_Q__

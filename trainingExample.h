
#ifndef	__CTRAINING_EXAMPLE_H__
#define __CTRAINING_EXAMPLE_H__

#include <stdio.h>	// file

// Ϊ�˼����Ƶ�ź��еĹؼ��Σ�ƽ���������ĶΣ�

class CTrainingExample {
private:

private:

public:
	void SamplesComing(short *samps, long iSampStartGlobal, int nNumSampsInBuffer);
	int CalcVector();
	bool IsCreatedOK();

public:
	CTrainingExample();
	virtual ~CTrainingExample();
};

#endif	// __CTRAINING_EXAMPLE_H__

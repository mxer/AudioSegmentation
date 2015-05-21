
#include "stdafx.h"	//yds

#include "ppsvm.h"

#ifndef __TRAINING_PHASE

CSvmPredict::CSvmPredict()
{
	pmodel_ = 0;
}

CSvmPredict::~CSvmPredict()
{
	if (pmodel_) {
        svmFreeAndDestroyModel(&pmodel_);
	}
}

// �� CSvmPredict ����������ڼ䣬���ܶ��װ�벻ͬģ�ͣ����ⲿ���ƵĽ�����֤����ѵ��+���ԣ�
bool CSvmPredict::LoadModel(const char *model_file)
{
	if (pmodel_) {
        svmFreeAndDestroyModel(&pmodel_);
	}
//
    pmodel_ = svmLoadModel(model_file);
	if (pmodel_ == NULL) {
        return false;
	}

    return true;
}

double CSvmPredict::predict(struct svm_node *x)
{
    return svm_predict(pmodel_, x);
}

#endif



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
#include <string>
#include <iostream>

#include "parameters.h"
#include "svmInterface.h"

void exit_input_error(int line_num);
void exit_with_help();

CSvmInterface::CSvmInterface(const char* psIn, const char* psOut)
{
	// ���������ļ���Ŀ¼
	m_pInRootPath = psIn;
	m_pOutRootPath = psOut;

#ifdef __TRAINING_PHASE
	m_pModel = NULL;
	mx_space = NULL;

#else
	m_pSvmNodes = NULL;

	m_pPredictor = new CSvmPredict();
	m_pScaler = new CSvmScale();	//yds : remove parameter "ndims"
#endif
}

CSvmInterface::~CSvmInterface()
{
#ifdef __TRAINING_PHASE
	char *psfn = Malloc(char, 260);

/*	sprintf(psfn, "%s%s", m_pOutRootPath, FN_TRAINING_DATA);
	remove(psfn);*/

	sprintf(psfn, "%s%s", m_pOutRootPath, FN_SCALED_DATA);
	remove(psfn);

	free(psfn);
#else
	if (m_pSvmNodes)
		free(m_pSvmNodes);
//
	if (m_pPredictor)
		delete m_pPredictor;
	if (m_pScaler)
		delete m_pScaler;
#endif
}

#ifdef __TRAINING_PHASE

int ReadTextLine(FILE *pfr, char*&, int&);

// �ӿں�����
// "psfn_trainingData" ѵ�������ļ�
// "ndim" ������������ά��
bool CSvmInterface::ppsvmTrain(const char *psfn_trainingData, int ndim)
{
// �������ò���

	// �γ� "m_param"
	// SVM �����ļ������룩
	char* psfn_config = Malloc(char, 260);
	sprintf(psfn_config, "%s%s", m_pInRootPath, FN_SVM_CONFIG);
	ParseConfigFile(psfn_config);
	free(psfn_config);
//

//	fileFormatTransform(psfn_trainingData, ndim);	//yds

// if no sacling, data file is just psfn_trainingData
//yds
	string fn_training_data(psfn_trainingData);
    if (m_param.tag[2]) {// ��Ҫ����������һ��
		printf("Scaling the data, ...\n");
        CSvmScale* psvm_s = new CSvmScale(ndim);	//CSvmScale(2000);
		string fn_scaled_data(m_pOutRootPath);
		fn_scaled_data += FN_SCALED_DATA;	// ��һ�����ѵ�����������ļ��������
		string fn_params(m_pOutRootPath);
		fn_params += FN_SCALE_PARAMS;	// ��һ�������ļ��������
		// ���һ������Ϊ���ݹ�һ�������ļ���������ļ������������ڶ�δ֪�����������ʱҪ��
        psvm_s->scaleDataSet(fn_training_data, fn_scaled_data, fn_params);
        // use this scaled data file as training data file
        fn_training_data = fn_scaled_data;
		delete psvm_s;
    }
//
//yds
	string fn_model(m_pOutRootPath);
	fn_model += FN_MODEL_BUILT;	// ����ģ���ļ��������
	// ������ѵ�����ݣ����룩�ļ�������Ҫ�����ģ���ļ���
    pp_svm_train(fn_training_data.c_str(), fn_model.c_str());
//
    return true;
}

// �� 2 �㺯����ֱ�ӱ��ӿں������ã�
void CSvmInterface::pp_svm_train(const char *input_file_name, const char *model_file_name)
{
	//char input_file_name[1024];
	//char model_file_name[1024];

/*	�Ƶ����ˣ��ӿں������ˣ�
	assert(g_psline == NULL);
    MaxLineLengthInBytes = 1024;
    g_psline = Malloc(char, MaxLineLengthInBytes);

	// �γ� "g_param"
	ParseConfigFile(svm_config_file);*/

	// �γ� "g_prob" ��  "x-space"
	ReadProblem(input_file_name);
	// �������������/ָ�룩��ָ���ǳ�����
	const char *perror_msg;
	perror_msg = svm_check_parameter(&m_prob, &m_param);

	if (perror_msg) {
		fprintf(stderr, "ERROR: %s\n", perror_msg);
		exit(1);
	}

	if (m_param.tag[0]) {// ��������֤ʵ�飬������
		fprintf(stderr, "\nDoing cross validation, # folds = %d\n", m_param.tag[1]);
		DoCrossValidation();
	} else {// ֻ��ѵ��ģ�ͣ�ȫ������������ѵ��ģ�ͣ�������������֤
		// we always save the model
//		fprintf(stderr,"now trainning the model, ...\n");
		m_pModel = SvmTrain(&m_prob, &m_param);	// "model" ÿ�ζ����´����ģ�

//		fprintf(stderr,"saving the model to a file, ...\n");
		if (svmSaveModel(model_file_name, m_pModel)) {
			fprintf(stderr, "can't save the model to file %s !\n", model_file_name);
			exit(1);
		}
		svmFreeAndDestroyModel(&m_pModel);	// ģ�ʹ����ļ����ͷ�ģ�ͣ����ÿ�ģ��ָ��
	}
	svmDestroyParam(&m_param);
	free(m_prob.pdbyy);
	free(m_prob.ppx);
	free(mx_space);
	mx_space = NULL;
}

// read in a problem (in svmlight format)
void CSvmInterface::ReadProblem(const char *filename)
{
	int elements, max_index, inst_max_index, iii, iNode;
	FILE *fp = fopen(filename, "rt");
	char *endptr;
	char *idx, *val, *label;

	if (fp == NULL) {
		fprintf(stderr,"can't open input file %s\n", filename);
		exit(1);
	}

	m_prob.ll = 0;	// ��������
	elements = 0;	// ������Ԫ�ظ���������������ʶ��

	int nLen = 1024;
	char* psTxtLine = Malloc(char, nLen);
	while (ReadTextLine(fp, psTxtLine, nLen) == 0) {
		char *pp = strtok(psTxtLine, " \t"); // label

		// features
		while(1) {
			pp = strtok(NULL, " \t");
			if (pp == NULL || *pp == '\n') // check '\n' as ' ' may be after the last feature
				break;
			++elements;
		}
		++elements;
		++m_prob.ll;
	}
	rewind(fp);

	m_prob.pdbyy = Malloc(double, m_prob.ll);
	m_prob.ppx = Malloc(struct svm_node*, m_prob.ll);	// ÿ������һ���ڵ����
	mx_space = Malloc(struct svm_node, elements);	// ÿ��Ԫ��һ���ڵ�

	max_index = 0;
	iNode = 0;
	for (iii=0; iii<m_prob.ll; iii++) {
		inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
		ReadTextLine(fp, psTxtLine, nLen);
		m_prob.ppx[iii] = &mx_space[iNode];
		label = strtok(psTxtLine, " \t\n");
		if (label == NULL) // empty line
			exit_input_error(iii+1);

		m_prob.pdbyy[iii] = strtod(label, &endptr);
		if(endptr == label || *endptr != '\0')
			exit_input_error(iii+1);

		while (1) {
			idx = strtok(NULL,":");
			val = strtok(NULL," \t");

			if(val == NULL)
				break;

			errno = 0;
			mx_space[iNode].index = (int) strtol(idx, &endptr, 10);
			if (endptr == idx || errno != 0 || *endptr != '\0' || mx_space[iNode].index <= inst_max_index)
				exit_input_error(iii+1);
			else
				inst_max_index = mx_space[iNode].index;

			errno = 0;
			mx_space[iNode].value = strtod(val, &endptr);
			if (endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_error(iii+1);

			iNode++;
		}

		if (inst_max_index > max_index)
			max_index = inst_max_index;
//
		mx_space[iNode++].index = -1;
	}

	if (m_param.gamma == 0 && max_index > 0)
		m_param.gamma = 1.0/max_index;
//
	if (m_param.kernel_type == PRECOMPUTED) {
		for (iii=0; iii<m_prob.ll; iii++) {
			if (m_prob.ppx[iii][0].index != 0) {
				fprintf(stderr, "Wrong input format: first column must be 0:sample_serial_number\n");
				exit(1);
			}
			if ((int)m_prob.ppx[iii][0].value <= 0 || (int)m_prob.ppx[iii][0].value > max_index) {
				fprintf(stderr, "Wrong input format: sample_serial_number out of range\n");
				exit(1);
			}
		}
	}

	fclose(fp);
	free(psTxtLine);	// ���ı��ڴ�
}

// �� 3 �㺯��
void CSvmInterface::DoCrossValidation()
{
	int ii;
	int total_correct = 0;
	double total_error = 0;
	double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
	double *pdbTarget = Malloc(double, m_prob.ll);	// ����ʶ����

	svmCrossValidation(&m_prob, &m_param, pdbTarget);
	if (m_param.svm_type == EPSILON_SVR || m_param.svm_type == NU_SVR) {
		for (ii=0; ii<m_prob.ll; ii++) {
			double y = m_prob.pdbyy[ii];
			double v = pdbTarget[ii];
			total_error += (v-y)*(v-y);
			sumv += v;
			sumy += y;
			sumvv += v*v;
			sumyy += y*y;
			sumvy += v*y;
		}
		printf("Cross Validation Mean squared error = %g\n", total_error/m_prob.ll);
		printf("Cross Validation Squared correlation coefficient = %g\n",
			((m_prob.ll*sumvy-sumv*sumy)*(m_prob.ll*sumvy-sumv*sumy))/
			((m_prob.ll*sumvv-sumv*sumv)*(m_prob.ll*sumyy-sumy*sumy))
			);
	} else {
		for (ii=0; ii<m_prob.ll; ii++) {
			if (pdbTarget[ii] == m_prob.pdbyy[ii])
				total_correct++;
		}
		printf("Cross Validation Accuracy = %g%%\n", 100.0*total_correct/m_prob.ll);
	}
	free(pdbTarget);
}

#else

#endif	// #ifndef __TRAINING_PHASE



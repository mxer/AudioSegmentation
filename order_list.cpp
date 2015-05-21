

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
#include "order_list.h"

// һ������Ͳ��뷽��
// 
void COrderedDoubles::NewValueCome(double dbval)
{
	// �Ҳ���㣬������
	int iElem;
	for (iElem=0; iElem<m_nNumElems; iElem++) {
		if (_bheadLarge) {// �Ӵ�С���򣬡�����
			if (dbval > _list[iElem]) break;
		} else {// ��С�������򣬡�����
			if (dbval < _list[iElem]) break;
		}
	}
	// "iElem" �����
	for (int iElem02=m_nNumElems-1; iElem02>=iElem; iElem02--) {// ֻ������Ԫ�أ�������
		if (iElem02+1 < m_nNumCanHold) {// ����ʱ�����һ��Ԫ�ز��ƣ��ʱ��壩
			_list[iElem02+1] = _list[iElem02];
		}
	}
	// �绹�п�λ�������
	if (iElem < m_nNumCanHold) {// ���п�λ��������
		_list[iElem] = dbval;
		if (m_nNumElems < m_nNumCanHold) {
			m_nNumElems++;
		}
	}
}

void COrderedDoubles::AverageVal(double& dbaverage)
{
	if (m_nNumElems == 0) return;

	dbaverage = 0.0;
	int iElem;
	for (iElem=0; iElem<m_nNumElems; iElem++) {
		dbaverage += _list[iElem];
	}
	dbaverage /= m_nNumElems;
}

double COrderedDoubles::AverageVal()
{
	assert(m_nNumElems > 0);

	double dbaverage = 0.0;
	int iElem;
	for (iElem=0; iElem<m_nNumElems; iElem++) {
		dbaverage += _list[iElem];
	}
	dbaverage /= m_nNumElems;

	return dbaverage;
}

// bheadLarge : ���򷽷���true, �Ӵ�С; false, ��С����
// 
COrderedDoubles::COrderedDoubles(int num_elems, bool bheadLarge)
{
	_list = Malloc(double, num_elems);
	if (_list) {
		m_nNumCanHold = num_elems;		
	} else {
		m_nNumCanHold = 0;
	}
	_bheadLarge = bheadLarge;
	m_nNumElems = 0;
}

// ����������ݣ������ͷ��ڴ�
// 
void COrderedDoubles::InitList()
{
	m_nNumElems = 0;
}

COrderedDoubles::~COrderedDoubles()
{
	if (_list)
		free(_list);
}


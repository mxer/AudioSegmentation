
#include "stdafx.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>	//  _S_IWRITE and _S_IREAD

#include "my_util.h"

// A return value of -1 indicates an error
int OpenFileWr(const char* psfn);
int OpenFileRd(const char* psfn);

// ���������
//		ѡ�εĵ�һ���������ź��е���ţ���
//		ѡ�ΰ����Ĳ�������
//		�źŲ�����
// ���������
//		���ɵġ�ѡ����ʼʱ���ѡ��ʱ�����ַ�����ŵ�λ�ã����治��С�� 64 ���ֽڣ���
// 
void StartPosAndLengthText(unsigned long sss, unsigned long lll, int srate, char *psOut)
{
	// �����Ĺؼ��ε���ʼλ�ã����룩�Ͷεĳ��ȣ����룩
	unsigned long ul_b;
	unsigned long ul = (double)sss/srate*1000;	// ms
	ul_b = ul/60000;
	sprintf(psOut, "%02d:", ul_b);	// minute
	ul = ul % 60000;

	ul_b = ul/1000;
	sprintf(psOut, "%s%02d:", psOut, ul_b); // second
	ul = ul % 1000;

	sprintf(psOut, "%s%03d; ", psOut, ul);	// ms

	// �εĳ��ȣ����룩��������
	ul = (double)lll/srate*1000;	// ms
	ul_b = ul/60000;
	sprintf(psOut, "%s%02d:", psOut, ul_b); // minute
	ul = ul % 60000;

	ul_b = ul/1000;
	sprintf(psOut, "%s%02d:", psOut, ul_b); // second
	ul = ul % 1000;

	sprintf(psOut, "%s%03d", psOut, ul);	// ms
}

int OpenFileWr(const char* psfn)
{
	int fhw = _open(psfn, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE );
	return fhw;
}
int OpenFileRd(const char* psfn)
{
	int fhr = _open(psfn, _O_RDONLY | _O_BINARY, 0);
	return fhr;
}

/*


*/

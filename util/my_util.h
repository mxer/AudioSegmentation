
#ifndef __MY_UTIL_H001__
#define __MY_UTIL_H001__

// ��ؼ��μ�ⴰ�ڵĿ�ȣ����ؼ��ο�ȣ����Դ������������ƣ�ÿ�����̶�Ϊ 30 ���룩��
void SetMaxMainSegSize(unsigned short usMaxMainSegSize);

// ���������
//		ѡ�εĵ�һ���������ź��е���ţ���
//		ѡ�ΰ����Ĳ�������
//		�źŲ�����
// ���������
//		���ɵġ�ѡ����ʼʱ���ѡ��ʱ�����ַ�����ŵ�λ�ã����治��С�� 64 ���ֽڣ���
// 
void StartPosAndLengthText(unsigned long ss, unsigned long ll, int srate, char *psOut);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>	// for "FILE *"

// WAV file header
typedef struct {  
	char			root_chunk_id[4];	/* 'RIFF' */
	unsigned long	root_chunk_data_size;	/* length of root chunk */
	char			riff_type_id[4];		/* 'WAVE' */
	char			fmt_chunk_id[4];		/* 'fmt ' */
	unsigned long	fmt_chunk_data_size;	/* length of sub_chunk, always 16 bytes */
	unsigned short	compression_code;		/* always 1 = PCM-Code */
	unsigned short	num_of_channels;		/* 1 = Mono, 2 = Stereo */
	unsigned long	sample_rate;	/* Sample rate */
	unsigned long	byte_p_sec;	/* average bytes per sec */
	unsigned short	byte_p_sample;	/* Bytes per sample */
	unsigned short	bit_p_sample;	/* bits per sample, 8, 12, 16 */
} waveheader_t;

class CWFFile {
private:
	waveheader_t m_header;	// �����ļ�ͷ

	FILE *m_pfWaveR;

public:
	int OpenWaveFile(const char *psfn_waveR);
	int MakeTargetSamplesData(int fhw, int&);
	void CloseWaveFile();

public:
	CWFFile();
	virtual ~CWFFile();
};

#endif	// __MY_UTIL_H001__



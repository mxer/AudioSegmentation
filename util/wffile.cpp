
#include "stdafx.h"

#include <io.h>
// for _O_RDONLY etc.
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <FLOAT.H>

#include "parameters.h"

#include "my_util.h"

#include "main_seg.h"

#define BUFFER_SIZE_IN_SAMPLES_DS	8192	// 8k, for down sampling op. 

typedef struct {  
	char			data_chunk_id[4];	/* 'data' */
	unsigned long	data_chunk_size;	/* length of data */
} waveheader_ext_t;


CWFFile::CWFFile()
{
	m_pfWaveR = NULL;
}

CWFFile::~CWFFile()
{
	if (m_pfWaveR)
		fclose(m_pfWaveR);
}

void CWFFile::CloseWaveFile()
{
	if (m_pfWaveR) {
		fclose(m_pfWaveR);
		m_pfWaveR = NULL;
	}
}

// ���ļ�ͷ��������ļ��Ƿ�Ϸ�
// ���������������Ϣ����ָ��ͻ����С���ֽ�����
// 
int CWFFile::OpenWaveFile(const char *psfn_waveR)
{
	m_pfWaveR = NULL;

	FILE *pfWave = fopen(psfn_waveR, "rb");	// read from a binary file
	if (pfWave == NULL) {
		printf("Can't open the wf file %s!\n", psfn_waveR);
		return -10;
	}

// �õ��ļ����ȣ�������
	fseek(pfWave, 0, SEEK_END);	// ָ���Ƶ��ļ�β
	long llen_data;
	llen_data = ftell(pfWave);
	fseek(pfWave, 0, SEEK_SET);	// ָ���ػ��ļ�ͷ
//

	fread(&m_header, sizeof(waveheader_t), 1, pfWave);
	if ( strncmp(m_header.root_chunk_id, "RIFF", 4) != 0 || strncmp(m_header.riff_type_id, "WAVE", 4) != 0) {
	// not a wave file
		fclose(pfWave);
		printf("Not a wave file, abort !\n");
		return -20;
	}
	if (m_header.fmt_chunk_data_size > 16) {
		fseek(pfWave, m_header.fmt_chunk_data_size-16, SEEK_CUR);
	}

	waveheader_ext_t header_ext;	// ��չ�ļ�ͷ
	fread(&header_ext, sizeof(waveheader_ext_t), 1, pfWave);
	long lLenWaveHeader = sizeof(waveheader_t)+m_header.fmt_chunk_data_size-16+sizeof(waveheader_ext_t);

	////////////////////////////////////////////////////////////////////////////////////////////////////

	llen_data -= lLenWaveHeader;

	printf("\nwave header length : %u\n", lLenWaveHeader);
	printf("wave data length : %u\n\n", llen_data);

//	char			root_chunk_id[4];		// 'RIFF'
//	unsigned long	root_chunk_data_size;	// length of root chunk
	printf("root_chunk_data_size : %u\n", m_header.root_chunk_data_size);
//	char			riff_type_id[4];		// 'WAVE'
//	char			fmt_chunk_id[4];		// 'fmt '
//	unsigned long	fmt_chunk_data_size;	// length of sub_chunk, always 16 bytes
	printf("fmt_chunk_data_size(16) : %u\n", m_header.fmt_chunk_data_size);
//	unsigned short	compression_code;		// always 1 = PCM-Code
	printf("compression_code(1) : %d\n", m_header.compression_code);

//	unsigned short	num_of_channels;		// 1 = Mono, 2 = Stereo
	printf("num_of_channels(1 = Mono, 2 = Stereo) : %d\n", m_header.num_of_channels);

//	unsigned long	sample_rate;			// Sample rate
	printf("sample_rate : %u\n", m_header.sample_rate);

//	unsigned long	byte_p_sec;				// average bytes per sec
	printf("byte_p_sec(average bytes per sec) : %u\n", m_header.byte_p_sec);

//	unsigned short	byte_p_sample;			// Bytes per sample, including the sample's data for all channels!
	printf("byte_p_sample : %d\n", m_header.byte_p_sample);
//	unsigned short	bit_p_sample;			// bits per sample, 8, 12, 16
	printf("bit_p_sample(8, 12, or 16) : %d\n", m_header.bit_p_sample);

//	char			data_chunk_id[4];		// 'data'
//	unsigned long	data_chunk_size;		// length of data
	printf("data_chunk_size : %u\n", header_ext.data_chunk_size);
	if ((unsigned long)llen_data != header_ext.data_chunk_size) {
		printf("llen_data = %u, not equal to data_chunk_size !\n", (unsigned long)llen_data);
//		assert(0);
	}

	if (m_header.compression_code != 1) {
		fclose(pfWave);
		printf("Not in the reqired compression code, abort !\n");
		return -30;
	}
	if (m_header.bit_p_sample != 16) {
		fclose(pfWave);
		printf("Bits per sample is not 16, abort !\n");
		return -40;
	}
	if (m_header.sample_rate < UNIFIED_SAMPLING_RATE) {
		fclose(pfWave);
		printf("Sampling rate is less than required, abort !\n");
		return -50;
	}
	if (m_header.num_of_channels > 2) {
		fclose(pfWave);
		printf("More than 2 channels, abort !\n");
		return -60;
	}

	// number of original samples in the audio file !!!
//	m_lNumSamplesInFile = m_header_ext.data_chunk_size/m_header.byte_p_sample;
	// number of selected samples
//	m_lNumSamplesInFile /= DOWN_SAMPLING_RATE;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	m_pfWaveR = pfWave;
	fseek(m_pfWaveR, lLenWaveHeader, SEEK_SET);	// ���ļ�ָ���Ƶ� Waveform ���ݿ�ʼ��

	return 0;	// OK
}

// ��ͳһ����������Ƿ���������������������������ֵ���ϲ���һ��������һ������ֵ��
// "maxAbs" �źŲ�������ֵ�����ֵ
int CWFFile::MakeTargetSamplesData(int fhw, int& maxAbs)
{
	assert(m_header.bit_p_sample == 16 || m_header.bit_p_sample == 8);
	// "m_header.sample_rate" : ԭʼ����Ƶ��
	// ԭʼ�����ʱ�����ڻ���� "UNIFIED_SAMPLING_RATE"���˼�Ŀ������ʣ�������
	if (m_header.sample_rate < UNIFIED_SAMPLING_RATE) {
		printf("m_header.sample_rate(%u) < UNIFIED_SAMPLING_RATE !\n", m_header.sample_rate);
		return -1;
	}

	short *NewlyMadeSampsBuff = NULL;
	unsigned char *szOrginalSampsBuffer = NULL;
	// buffer to hold newly made down-sampling samples, 16 bits per sample!
	NewlyMadeSampsBuff = (short *)malloc(sizeof(short)*BUFFER_SIZE_IN_SAMPLES_DS+
										(m_header.byte_p_sample)*BUFFER_SIZE_IN_SAMPLES_DS);
	if (NewlyMadeSampsBuff == NULL) {
		printf("No memory !\n");
		return -2;
	}
	szOrginalSampsBuffer = (unsigned char *)(NewlyMadeSampsBuff+BUFFER_SIZE_IN_SAMPLES_DS);

// ////////////////////////////////////////////////////////////////////////////////////////

	maxAbs = 0;
	int xxx;	// OK

// ע�⣺Ƿ�����źź�ԭ�����źŶ�Ӧ��ʱ����һ���ģ�����

	// ��ԭ�������桱�е�һ��λ������Ĳ�����ȫ���±�
	long iOriginalSampStartG = 0;
	// �������ɵĵ�һ��Ƿ�����㣨����Ƿ�������桱�е�һ���㣩��ȫ���±�
	long iTargetSampStartG = 0;
	// total number of down-sampling samples made 
	unsigned long nNumNewlyMadeSampsTotal = 0;

#if defined(__USE_WHOLE_SIGNAL)
	int& sr = xxx;
	sr = UNIFIED_SAMPLING_RATE;
	_write(fhw, &sr, sizeof(int));	// ������
	_write(fhw, &iTargetSampStartG, sizeof(unsigned long));	// ��ʼ�����±�
	_write(fhw, &nNumNewlyMadeSampsTotal, sizeof(unsigned long));	// Ŀ���������
#else
	CMainSegment* pMainSeg = new CMainSegment();
	if (pMainSeg == NULL || !pMainSeg->IsCreatedOK()) {
		printf("pMainSeg == NULL || !pMainSeg->IsCreatedOK() !\n");
		free(NewlyMadeSampsBuff);
		return -3;
	}
	// �ؼ����޳�������
	xxx = NumFrameSkipsInUnitM*NumSamplesInFrameSkipM+NumSamplesInFrameM;	// �źŲ�������
	xxx = (double)xxx/UNIFIED_SAMPLING_RATE*1000.0/MILISECONDS_DETECT_SKIP;	// ����ɼ����������
	pMainSeg->SetMaxMainSegSize(xxx);
#endif

	xxx = 0;
	// ÿ��ԭʼ���������ļ�����һ����ԭ�������ݣ�������һ��Ƿ������
	while ( !feof(m_pfWaveR) ) {
		int nNumNewlyMadeSamps = 0;	// ���������ӱ��ζ�ȡ��ԭʼ�ź����ݣ����ɵ�Ƿ���������
		//��һ���ӵ�ǰ������Ƶ�����ļ���һ�����ݣ�������
		size_t uNumOrigSampsRead;
		uNumOrigSampsRead = fread(szOrginalSampsBuffer, 
					m_header.byte_p_sample,	// һ����������������һ������ʱ�����������������
					BUFFER_SIZE_IN_SAMPLES_DS,	// Ҫ��ȡ�Ĳ�������
					m_pfWaveR);
		if (uNumOrigSampsRead == 0) {// no content read
			break;
		}

		if (iTargetSampStartG == 0) {
		// ���������Ƿ��������һ��ԭʼ��������Ҫֱ�ӿ����ġ�Ҳ����˵��Ƿ�����źŵĵ�һ�����������ԭ�źŵĵ�һ��
		// �����㣡
			assert(nNumNewlyMadeSamps == 0);
			// ��һ������ֵ�հ�
// clrc_001
#ifdef __COMBINE_L_R_CHANNELS
// ȡ����������ƽ��
			if (m_header.num_of_channels > 1) {
				long lval;	// �� "long" ��֤������̲����
				if (m_header.bit_p_sample == 16) {
					lval = *((short *)szOrginalSampsBuffer);
					lval += *(((short *)szOrginalSampsBuffer)+1);
				} else if (m_header.bit_p_sample == 8) {
					lval = *((char *)szOrginalSampsBuffer);
					lval += *(((char *)szOrginalSampsBuffer)+1);
					lval *= 256;
				}
				lval /= 2;
				NewlyMadeSampsBuff[nNumNewlyMadeSamps] = lval;
			} else {// ֻ��һ������ʱ��������
				if (m_header.bit_p_sample == 16) {
					NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((short *)szOrginalSampsBuffer);
				} else if (m_header.bit_p_sample == 8) {
					NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((char *)szOrginalSampsBuffer);
					NewlyMadeSampsBuff[nNumNewlyMadeSamps] *= 256;
				}
			}
#else
// ֻȡ��һ������
			if (m_header.bit_p_sample == 16) {
				NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((short *)szOrginalSampsBuffer);	// ��һ������ֵ�հ�
			} else if (m_header.bit_p_sample == 8) {
				NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((char *)szOrginalSampsBuffer);	// ��һ������ֵ�հ�
				NewlyMadeSampsBuff[nNumNewlyMadeSamps] *= 256;
			}
#endif
// clrc_001
			// �Ҳ����ľ���ֵ���ֵ
			if (maxAbs < abs(NewlyMadeSampsBuff[nNumNewlyMadeSamps]))
				maxAbs = abs(NewlyMadeSampsBuff[nNumNewlyMadeSamps]);
			nNumNewlyMadeSamps++;
		}

		//���������ɶ�Ӧ���²�����������
		while (1) {
		// �����ǽ�Ƿ�������Ӧ��ĳ��ԭ�����㣨ֻ��������㣩�����������ڵ�ԭ�����㣨���������������㣩֮�䣡����

			// ������Ƿ�������Ӧ��ԭʼ�������ȫ���±꣨�����ϲ�һ����������
			long iOriginalSampG;
			long iTargetSampG;
			// ����Ƿ�������±꣬������
			iTargetSampG = iTargetSampStartG+nNumNewlyMadeSamps;
			// �������Ӧ��ԭ�������±�
			iOriginalSampG = (double)iTargetSampG*m_header.sample_rate/UNIFIED_SAMPLING_RATE+0.5;
			if (iOriginalSampG > iOriginalSampStartG+uNumOrigSampsRead-1) {
			// ��δ������Ŀ��ԭ�����㡱��������
				iOriginalSampStartG += uNumOrigSampsRead;
				break;
			}
			// Ŀ��ԭ�������ڻ����е�λ��
			const unsigned char *puchar = szOrginalSampsBuffer+
				(m_header.byte_p_sample)*(iOriginalSampG-iOriginalSampStartG);
// clrc_002
#ifdef __COMBINE_L_R_CHANNELS
// ȡ����������ƽ��
			if (m_header.num_of_channels > 1) {
				long lval;
				if (m_header.bit_p_sample == 16) {// 16 λ����
					lval = *((short *)puchar);	// �������ĵ�һ������
					lval += *(((short *)puchar)+1);	// �������ĵڶ�������
				} else if (m_header.bit_p_sample == 8) {// 8 λ����
					lval = *((char *)(puchar));	// �������ĵ�һ������
					lval += *(((char *)puchar)+1);	// �������ĵڶ�������
					lval *= 256;
				}
				lval /= 2;
				NewlyMadeSampsBuff[nNumNewlyMadeSamps] = lval;
			} else {// ֻ��һ������ʱ��������
				if (m_header.bit_p_sample == 16) {
					NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((short *)puchar);
				} else if (m_header.bit_p_sample == 8) {
					NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((char *)puchar);
					NewlyMadeSampsBuff[nNumNewlyMadeSamps] *= 256;
				}
			}
#else
// ֻȡ��һ������
			if (m_header.bit_p_sample == 16) {
				NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((short *)puchar);
			} else if (m_header.bit_p_sample == 8) {
				NewlyMadeSampsBuff[nNumNewlyMadeSamps] = *((char *)puchar);
				NewlyMadeSampsBuff[nNumNewlyMadeSamps] *= 256;
			}
#endif
// clrc_002
			// �Ҳ����ľ���ֵ���ֵ
			if (maxAbs < abs(NewlyMadeSampsBuff[nNumNewlyMadeSamps]))
				maxAbs = abs(NewlyMadeSampsBuff[nNumNewlyMadeSamps]);
			nNumNewlyMadeSamps++;	// ����Ƿ������������
			
		}	// end of "while (1) {"
#if defined(__USE_WHOLE_SIGNAL)
		//�����������������ɵĲ���������Ƿ�������������ϲ���Ĳ������ݣ�д������ļ�
		if (_write(fhw, NewlyMadeSampsBuff, nNumNewlyMadeSamps*sizeof(short)) != nNumNewlyMadeSamps*sizeof(short)) {
			xxx = -1;
			break;
		}
#else
		pMainSeg->DetectInWindow(NewlyMadeSampsBuff, iTargetSampStartG, nNumNewlyMadeSamps);
#endif	// #if defined(__USE_WHOLE_SIGNAL)

		//���ģ�׼���������ļ�������һ��ԭ�������ݣ�������
		iTargetSampStartG += nNumNewlyMadeSamps;
		nNumNewlyMadeSampsTotal += nNumNewlyMadeSamps;
	}	// end of "while ( !feof(m_pfWaveR) ) {"
//
	free(NewlyMadeSampsBuff);
//
#if defined(__USE_WHOLE_SIGNAL)
	_lseek(fhw, sizeof(int)+sizeof(unsigned long), SEEK_SET);	// ���ļ�ͷ������ǰ��������
	_write(fhw, &nNumNewlyMadeSampsTotal, sizeof(unsigned long));	// ��������
#else
	if (pMainSeg) {
		pMainSeg->WriteMainSegToFile(fhw, NULL, NULL);
		delete pMainSeg;
	}
#endif
	printf("Length of clip in seconds : %f\n", (double)nNumNewlyMadeSampsTotal/UNIFIED_SAMPLING_RATE);

	return xxx;	// OK
}


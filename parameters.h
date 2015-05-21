
#ifndef	__PARAMETERS_H__
#define __PARAMETERS_H__
#include "mydefs.h"

// ��һ����Ƶ�Ĺؼ���ʱ����ⴰ��������С
#define MILISECONDS_DETECT_SKIP		30.0
// �ؼ��μ��ʱ������������ȣ�����ƣ��������ļ�����
#define MINIMUM_MUSIC_LENGTH_ALLOWED_IN_SECONDS		0.64	// �� "MINIMUM_NUM_FRAMES_IN_UNIT" ��Ӧ������

// �ź�ֵ�ľ���ֵƽ��ֵС�����ֵ��Ϊ�����������ᱻ����ͷȥβ����
#define MINIMUM_MEAN_ABS_SIGNAL_VALUE		81.9175	// 32767(the largest abs value of signal)*0.25%

// ////////////////////////////////////////////////////////////////////////////////////////////////

#define UNIFIED_SAMPLING_RATE		22050.0		// ͳһ����Ƶ�ʣ�Ŀ������ʣ�

#define	NumBinsInFftWinM			512	//1024
#define NumSamplesInFrameM			512	//882
#define	NumSamplesInFrameSkipM		441	// ����С�ڵ��� "NumSamplesInFrameM" ������

#define MILISECONDS_PER_FRAME_SKIP	20

#define NUMBER_OF_OCTAVES			7

//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

/*
#define CCCC_FACTOR				(1.0E+3)
*/

// 2013/02/27 ���� 10 άЧ���Ժõ�
#define NumDimsAreaMomentsM		10	//15

#define NumDimsMfccM			13
#define NUM_MEL_BANDS			30	// number of Mel bands

#define NUM_DIMS_LPC			13

#ifdef __USE_STE_FEATURES
#define NUM_STE_FEATURES		5	//4
#endif

#ifdef __USE_SPECTRAL_SHAPE_FEATURES
#define NUM_DIMS_SPECTRAL_SHAPE		14	//7	//4
#endif

#ifdef __USE_BEAT_HISTOGRAM
#define NUM_DIMS_BH				13	//10
#endif

#ifdef __USE_MODULATION_SPECTRUM
#define NUM_DIMS_MSP			25
#endif	// __USE_MODULATION_SPECTRUM

#ifdef __USE_ZERO_CROSSING
#define NUM_DIMS_ZC				1
#endif	// __USE_ZERO_CROSSING

#ifdef __USE_SUB_BAND_ENERGY
#ifdef __USE_MEL_BAND_VALUE_AS_AM_SOURCE
#define NUM_DIMS_SBE			13
#else
#define NUM_DIMS_SBE			NUMBER_OF_OCTAVES
#endif	// __USE_MEL_BAND_VALUE_AS_AM_SOURCE
#endif	// __USE_SUB_BAND_ENERGY

#if defined(__USE_BEAT_HISTOGRAM) || defined(__USE_MODULATION_SPECTRUM)
#define BH_FFT_WIN_WIDTH		64	// ������ 2 ���������η�
#endif

// AM Դ���ݾ����а�����֡��
#define NumFramesAreaMomentsSrcM	8

// ע�⣺���ֵ������ "NumBinsInFftWinM" ��һ�룡����
#define WIDTH_SOURCE_DATA_AM		256

//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// ѵ���ͷ��������ͬ���Ĳ���ֵ��������һЩ��
// һ����Ԫ�а�����֡��������
#define NumFrameSkipsInUnitM		60	// ֡����Ϊ 20 ���룬�� 1.2 ��

#ifdef __TRAINING_PHASE
// ��һ����Ƶ�ļ��������ɶ��ѵ��������ÿ����Ԫ��Ӧһ��ѵ��������
#define NumFrameSkipsInUnitSkipM	60	// ��Ԫ������С����֡�������ƣ���֡����Ϊ 20 ���룬�� 1.2 ��
#define MINIMUM_NUM_FRAMES_IN_UNIT	30	// ֡����ѵ�������ĳ��Ȳ���С�ڴ�ֵ��������������
#else	// __TRAINING_PHASE
#define NumFrameSkipsInUnitSkipM	60	//20	// ��Ԫ������С����֡�������ƣ���֡����Ϊ 20 ���룬�� 0.4 ��
#define MAX_NUM_UNITS				9000	// ���ֶ���Ƶ�������ƣ�60��*60��/��*1000����/�룩/��20����/֡*20֡��

// ��ô��� units ���ǵķ�Χ������Ӧ��ʱ����
//	NumFrameSkipsInUnitM+NumFrameSkipsInUnitSkipM*(MINMUN_NUM_UNITS_IN_SEGMENT-1)
//	��������ز���ֵ�ĵ�ǰ���ã�Ϊ 60+20*(3-1) = 100 ֡������ 2.00 ��
// �γ�С�����ֵ���ͱ���Ϊ�������������������ڶ�
#define MINMUN_NUM_FRAMES_IN_SEGMENT			100	//3
// �����εĳ��ȣ��Ե�Ԫ���������ƣ��������ֵ���ű��������򽫱��ϲ������ڶ�
#define NUM_FRAMES_IN_SILENT_SEGMENT_TO_SURVIVE	160	//8

// ��Ŵ��ֶ���Ƶ��֡����ֵ���ļ�
#define FN_TEMP_STE		"~ste"

#endif	// #ifdef __TRAINING_PHASE

// һ����Ԫ�У�������εĳ��ȴ��ڵ�Ԫ�ܳ���һ������ʱ������Ϊ������Ԫ
#define RATIO_PERCENT_SILENT_LENGTH		0.8

// ��ֵ�趨�ӽ���ЧƵ�ʷ�Χ��Ϊ����Ƶ������
#ifdef __USE_MEL_BAND_VALUE_AS_AM_SOURCE
#define NUM_DIMS_AM_SORCE_DATA			NUM_MEL_BANDS
#else
#define NUM_DIMS_AM_SORCE_DATA			NUMBER_OF_OCTAVES
#endif	// __USE_MEL_BAND_VALUE_AS_AM_SOURCE

#endif	// __PARAMETERS_H__



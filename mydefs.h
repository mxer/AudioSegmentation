
#ifndef __MY_DEFINITIONS__
#define __MY_DEFINITIONS__

#define Malloc(type, n) (type *)malloc(sizeof(type)*(n))

#ifndef MY_PI
#define MY_PI		(3.1415926535897932384626433832795)
#endif

#ifndef TWO_PI
#define TWO_PI		(6.283185307179586476925286766559)
#endif

#ifndef MY_CPXV_TYPE
#define MY_CPXV_TYPE
typedef struct _cpxv_t {
	double re;
	double im;
} cpxv_t;
#endif

//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define __NORMALIZE_AUDIO_SAMPLES

//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ѡ����Щ������

#define __USE_STE_FEATURES

#define __USE_BEAT_HISTOGRAM

#define __USE_SPECTRAL_SHAPE_FEATURES

/*
#define __USE_MODULATION_SPECTRUM

#define __USE_ZERO_CROSSING

#define __USE_SUB_BAND_ENERGY
*/

//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ѵ���׶λ���ʶ��׶Σ�

/*
// ������ѵ���׶Σ������Ƿ��ࣨ��Ƶ�����ֶΣ��׶�
#define __TRAINING_PHASE
*/
// ��ʲô������ AM �����Դ���ݣ�
//#define __USE_MEL_BAND_VALUE_AS_AM_SOURCE
#ifndef __USE_MEL_BAND_VALUE_AS_AM_SOURCE
#define __USE_OCTAVE_BAND_VALUE_AS_AM_SOURCE
#endif	// __USE_MEL_BAND_VALUE_AS_AM_SOURCE

//
///////////////////////////////////////////////////////////////////////////////////////////////////
//

// ��Ƶ�ź��ļ����м��ļ���
#define FN_TEMP_WAVEFORM	"~wav.bin"
#ifdef __NORMALIZE_AUDIO_SAMPLES
#define FN_TEMP_WAVEFORM02	"~wav02"
#endif

//
//////////////////////////////////////////////////////////////////////////////
//

#ifdef __TRAINING_PHASE
/*
#define __GENERATE_WEKA_TRAINING_FILE
*/

#define __USE_WHOLE_SIGNAL

#define FN_TRAINING_EXAMPLES_LIST	"examples_list.txt"

// ѵ�����������ļ�
#ifdef __GENERATE_WEKA_TRAINING_FILE
#define FN_TRAINING_DATA_ORGINAL	"weka_train.arff"
#else
#define FN_TRAINING_DATA_ORGINAL	"~train.txt"
#endif	// __GENERATE_WEKA_TRAINING_FILE

#else	// #ifdef __TRAINING_PHASE

#define __USE_WHOLE_SIGNAL	// a must !!!

#endif	// #ifdef __TRAINING_PHASE

//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#endif	// __MY_DEFINITIONS__

/*

// SVM ��ʱ�ļ�Ŀ¼�������Ŀ¼�£�
#define SUB_DIR_SVM					"files/"
// SVM �����ļ�Ŀ¼���ڵ�ǰ����Ŀ¼�£�
#define SUB_DIR_CONFIG_SVM			"files/"

// opopop-20012
#if !defined(__MIREX_2012) || defined(__MIREX_2012_TEST_VERSION)

// ע�⣺���� 4 �����壨4 �������ֻ��ȡ��һ������
//#define __EMOTION_500_CN
#define __GENRE_1000_EN
//#define __EMOTION_2217_CN
//#define __EMOTION_APM_WAV_337
//

// �����������������������������������¼��������������ŵ��ļ�
#ifdef __EMOTION_500_CN
	#define NUM_EXAMPLES_MAX		500
	#define NUM_SONG_CLASSES		4
	#define NUM_FOLDS				3
	#define STR_FN_EXAMP_NAME_CLASS_No_FILE		"top500NewList4CN.txt"
#else
	#ifdef __GENRE_1000_EN
		#define NUM_EXAMPLES_MAX		1000
		#define NUM_SONG_CLASSES		10
		#define NUM_FOLDS				3
		#define STR_FN_EXAMP_NAME_CLASS_No_FILE		"genre_1000_songs.txt"
	#else
		#ifdef __EMOTION_2217_CN
				#define NUM_EXAMPLES_MAX		2217
				#define NUM_SONG_CLASSES		3
				#define NUM_FOLDS				5
				#define STR_FN_EXAMP_NAME_CLASS_No_FILE		"song_pp.txt"
		#else
		// __EMOTION_APM_WAV_337
				#define NUM_EXAMPLES_MAX		337
				#define NUM_SONG_CLASSES		5
				#define NUM_FOLDS				3
				#define STR_FN_EXAMP_NAME_CLASS_No_FILE		"APM_WAV_337.txt"
		#endif
	#endif
#endif

#endif
// opopop-20012

*/

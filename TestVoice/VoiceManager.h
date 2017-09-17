#ifndef _VOICEMANAGER_H__
#define _VOICEMANAGER_H__

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <alsa/asoundlib.h>  

#define VOICE_FILE_PATH "/home/opt/data/voice"


typedef long long	   off64_t;  
typedef unsigned char  uint8_t;  
typedef unsigned short uint16_t;  
typedef unsigned int   uint32_t;  


typedef struct SNDPCMContainer 
{  
    snd_pcm_t			*handle;			//文件描述符
    snd_output_t		*log;				//输出日志
    snd_pcm_uframes_t	chunk_size;			//块大小
    snd_pcm_uframes_t	buffer_size;		//缓冲区大小
    snd_pcm_format_t	format;				//文件格式
    uint16_t			channels;			//通道：单声道，立体声
    size_t				chunk_bytes;		//chunk_bytes就是我们单次从WAV读PCM数据的大小
    size_t				bits_per_sample;	//每个样本的比特数
    size_t				bits_per_frame;		//每一帧的比特数
    uint8_t				*data_buf;			//数据缓冲区
} SNDPCMContainer_t;  


//大小端模式
#if __BYTE_ORDER == __LITTLE_ENDIAN  
#define		COMPOSE_ID(a,b,c,d)		((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))  
#define		LE_SHORT(v)				(v)  
#define		LE_INT(v)               (v)  
#define		BE_SHORT(v)				bswap_16(v)  
#define		BE_INT(v)               bswap_32(v)  
#elif __BYTE_ORDER == __BIG_ENDIAN  
#define		COMPOSE_ID(a,b,c,d)		((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))  
#define		LE_SHORT(v)				bswap_16(v)  
#define		LE_INT(v)               bswap_32(v)  
#define		BE_SHORT(v)				(v)
#define		BE_INT(v)               (v)  
#else  
#error		"Wrong endian"  
#endif  

/*chunk是组成RIFF文件的基本单元，它的结构如下：
struct chunk
{
　　u32	id;				//块标志 
　　u32	size;			//块大小 
　　u8	dat[size];		//块内容 
};*/

//用到的ID
#define		WAV_RIFF        COMPOSE_ID('R','I','F','F')  
#define		WAV_WAVE        COMPOSE_ID('W','A','V','E')  
#define		WAV_FMT         COMPOSE_ID('f','m','t',' ')  
#define		WAV_DATA        COMPOSE_ID('d','a','t','a')  



/* WAVE fmt block constants from Microsoft mmreg.h header */  
#define		WAV_FMT_PCM					0x0001  
#define		WAV_FMT_IEEE_FLOAT			0x0003  
#define		WAV_FMT_DOLBY_AC3_SPDIF		0x0092  
#define		WAV_FMT_EXTENSIBLE			0xfffe  



/* Used with WAV_FMT_EXTENSIBLE format */  
#define WAV_GUID_TAG        "/x00/x00/x00/x00/x10/x00/x80/x00/x00/xAA/x00/x38/x9B/x71"  
  
/* it's in chunks like .voc and AMIGA iff, but my source say there 
   are in only in this combination, so I combined them in one header; 
   it works on all WAVE-file I have 
 */  


//这个结构体是wav文件标准RIFF的头信息
/*
RIFF WAVE Chunk
    ==================================
    |       |所占字节数|  具体内容   |
    ==================================
    | ID    |  4 Bytes |   'RIFF'    |
    ----------------------------------
    | Size  |  4 Bytes |             |
    ----------------------------------
    | Type  |  4 Bytes |   'WAVE'    |
    ----------------------------------
*/
typedef struct WAVHeader 
{  
    uint32_t	magic;				/* 'RIFF' */   //RIFF chunk 的ID
    uint32_t	length;				/* filelen */  //文件的大小，是结构体的长度 - 8个字节（从下一个字节开始算起的整个文件的大小）
    uint32_t	type;				/* 'WAVE' */	//wav文件的格式辨别码
} WAVHeader_t;  
  

/*
Format Chunk
    ====================================================================
    |               |   字节数  |              具体内容                |
    ====================================================================
    | ID            |  4 Bytes  |   'fmt '                             |
    --------------------------------------------------------------------
    | Size          |  4 Bytes  | 数值为16或18，18则最后又附加信息     |
    --------------------------------------------------------------------  ----
    | FormatTag     |  2 Bytes  | 编码方式，一般为0x0001               |     |
    --------------------------------------------------------------------     |
    | Channels      |  2 Bytes  | 声道数目，1--单声道；2--双声道       |     |
    --------------------------------------------------------------------     |
    | SamplesPerSec |  4 Bytes  | 采样频率                             |     |
    --------------------------------------------------------------------     |
    | AvgBytesPerSec|  4 Bytes  | 每秒所需字节数                       |     |===> WAVE_FORMAT
    --------------------------------------------------------------------     |
    | BlockAlign    |  2 Bytes  | 数据块对齐单位(每个采样需要的字节数) |     |
    --------------------------------------------------------------------     |
    | BitsPerSample |  2 Bytes  | 每个采样需要的bit数                  |     |
    --------------------------------------------------------------------     |
    |               |  2 Bytes  | 附加信息（可选，通过Size来判断有无） |     |
    --------------------------------------------------------------------  ----
*/
//这个结构体是wav文件格式标准，变量顺序是固定的
//这个就是ID为fmt的chunk的结构体，这个结构体存在于RIFF之后（wav文件就是固定顺序）
typedef struct WAVFmt 
{  
    uint32_t	magic;				/* 'FMT '*/		//fmt chunk 的ID
    uint32_t	fmt_size;			/* 16 or 18 */					//从下一个变量的字节数（18就是多了两个字节的附加信息）
    uint16_t	format;				/* see WAV_FMT_* */				///编码格式，声音的格式代号，例如WAVE_FORMAT_PCM，WAVE_F0RAM_ADPCM等等
    uint16_t	channels;											//单声道或立体声
    uint32_t	sample_rate;		/* frequence of sample */		//采样速率
    uint32_t	bytes_p_second;										//记录每秒的数据
    uint16_t	blocks_align;		/* samplesize; 1 or 2 bytes */  //记录区块的对齐单位 
    uint16_t	sample_length;		/* 8, 12 or 16 bit */		    //记录每个取样所需的位元数
} WAVFmt_t;  
  

//这个结构体是wav数据chunk
/*
Data Chunk
    ==================================
    |       |所占字节数|  具体内容   |
    ==================================
    | ID    |  4 Bytes |   'data'    |
    ----------------------------------
    | Size  |  4 Bytes |             |
    ----------------------------------
    | data  |          |             |
    ----------------------------------
*/
typedef struct WAVChunkHeader 
{  
    uint32_t	type;				/* 'data' */  
    uint32_t	length;				/* samplecount */  
} WAVChunkHeader_t;  
  


/*
wav文件格式
------------------------------------------------
|             RIFF WAVE Chunk                  |
|             ID  = 'RIFF'                     |
|             RiffType = 'WAVE'                |
------------------------------------------------
|             Format Chunk                     |
|             ID = 'fmt '                      |
------------------------------------------------
|             Fact Chunk(optional)             |
|             ID = 'fact'                      |
------------------------------------------------
|             Data Chunk                       |
|             ID = 'data'                      |
------------------------------------------------
*/

//设计一个容器，该容器包括了wav文件信息chunk和data chunk
typedef struct WAVContainer 
{  
    WAVHeader_t			header;		//RIFF
    WAVFmt_t			format;		//WAV 文件信息
    WAVChunkHeader_t	chunk;		//WAV数据
} WAVContainer_t;  
  

typedef struct WAVFmtExtensible 
{  
    WAVFmt_t	format;  
    uint16_t	ext_size;  
    uint16_t	bit_p_spl;  
    uint32_t	channel_mask;  
    uint16_t	guid_format;		/* WAV_FMT_* */  
    uint8_t		guid_tag[14];		/* WAV_GUID_TAG */  
} WAVFmtExtensible_t;  
  




enum VoiceSignalId
{
	NO_SIGNAL			= 0,	//无语音信号
	TURN_ON_DEVICE,				//开机语音提示
	WARNING_DANGER,				//报警：danger
	WARNING_DILONG_ONCE,		//报警：长滴声1声
	WARNING_DILONG_3TIMES,		//报警：长滴声3声
	WARNING_DISHORT,			//报警：短滴声1声
	WARNING_DIHIT,				//报警：超短滴1声
	WARNING_OVERSPEED,			//超速语音报警
};





class CVoiceManager
{
public:

	CVoiceManager();
	~CVoiceManager();
	static CVoiceManager* GetInstance();
	bool HasInstance();
	void Destory();
	bool VoiceInit();

	ssize_t SNDWAV_P_SaveRead(int fd, void *buf, size_t count);
	void SNDWAV_Play(SNDPCMContainer_t * sndpcm, WAVContainer_t * wav, int fd);


	int WAV_ReadHeader(int fd, WAVContainer_t *container);
	int WAV_WriteHeader(int fd, WAVContainer_t *container);
	ssize_t SNDWAV_ReadPcm(SNDPCMContainer_t *sndpcm, size_t rcount);
	ssize_t SNDWAV_WritePcm(SNDPCMContainer_t *sndpcm, size_t wcount);
	int SNDWAV_SetParams(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav);
	
private:
	int mFd;
	static CVoiceManager* mInstance;
	SNDPCMContainer_t mPlayback;
	WAVContainer_t mWav;



};





#endif



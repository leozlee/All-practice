//File   : lplay.c  
//Author : Loon <sepnic@gmail.com>  
  
#include <stdio.h>  
#include <malloc.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <string.h>  
#include <getopt.h>  
#include <fcntl.h>  
#include <ctype.h>  
#include <errno.h>  
#include <limits.h>  
#include <time.h>  
#include <locale.h>  
#include <sys/unistd.h>  
#include <sys/stat.h>  
#include <sys/types.h>  
#include <alsa/asoundlib.h>  
#include <assert.h>  
#include "wav_parser.h"  
#include "sndwav_common.h"  
  
ssize_t SNDWAV_P_SaveRead(int fd, void *buf, size_t count)  
{  
    ssize_t result = 0, res;  
  
    while (count > 0) 
	{  
        if ((res = read(fd, buf, count)) == 0)  
            break;  
        if (res < 0)  
            return result > 0 ? result : res;  
        count -= res;  
        result += res;  
        buf = (char *)buf + res;  
    }  
    return result;  
}  
  
void SNDWAV_Play(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav, int fd)  
{  
    int load, ret;  
    off64_t written = 0;  
    off64_t c;  
    off64_t count = LE_INT(wav->chunk.length);  
  
    load = 0;  
    while (written < count) 
	{  
        /* Must read [chunk_bytes] bytes data enough. */  
        do {  
            c = count - written;  
            if (c > sndpcm->chunk_bytes)  
                c = sndpcm->chunk_bytes;  
            c -= load;  
  
            if (c == 0)  
                break;  
            ret = SNDWAV_P_SaveRead(fd, sndpcm->data_buf + load, c);  
            if (ret < 0) {  
                fprintf(stderr, "Error safe_read/n");  
                exit(-1);  
            }  
            if (ret == 0)  
                break;  
            load += ret;  
        } while ((size_t)load < sndpcm->chunk_bytes);  
  
        /* Transfer to size frame */  
        load = load * 8 / sndpcm->bits_per_frame;  
        ret = SNDWAV_WritePcm(sndpcm, load);  
        if (ret != load)  
            break;  
          
        ret = ret * sndpcm->bits_per_frame / 8;  
        written += ret;  
        load = 0;  
    }  
}  
  
int main(int argc, char *argv[])  
{  
    char *filename;  
    char *devicename = "default";  //pcm默认文件设备
    int fd;  
    WAVContainer_t wav;  
    SNDPCMContainer_t playback;  
      
    if (argc != 2) {  
        fprintf(stderr, "Usage: ./lplay <FILENAME>/n");  
        return -1;  
    }  
     
	//结构体清零
    memset(&playback, 0x0, sizeof(playback));  
  

	//打开音频文件
    filename = argv[1];  
    fd = open(filename, O_RDONLY);  
    if (fd < 0) {  
        fprintf(stderr, "Error open [%s]/n", filename);  
        return -1;  
    }  
      
	//读wav文件的信息及数据，保存到wav结构上
    if (WAV_ReadHeader(fd, &wav) < 0) 
	{  
        fprintf(stderr, "Error WAV_Parse [%s]/n", filename);  
        goto Err;  
    }  
   //将错误输出信息重定向到指定位置
    if (snd_output_stdio_attach(&playback.log, stderr, 0) < 0) 
	{  
        fprintf(stderr, "Error snd_output_stdio_attach/n");  
        goto Err;  
    }  
	//alsa打开设别函数，回放模式
    if (snd_pcm_open(&playback.handle, devicename, SND_PCM_STREAM_PLAYBACK, 0) < 0) 
	{  
        fprintf(stderr, "Error snd_pcm_open [ %s]/n", devicename);  
        goto Err;  
    }  
	//设置参数
    if (SNDWAV_SetParams(&playback, &wav) < 0) 
	{  
        fprintf(stderr, "Error set_snd_pcm_params/n");  
        goto Err;  
    }  
    snd_pcm_dump(playback.handle, playback.log);  
  
	//播放文件
    SNDWAV_Play(&playback, &wav, fd);  
    snd_pcm_drain(playback.handle);  
  
    close(fd);  
    free(playback.data_buf);  
    snd_output_close(playback.log);  
    snd_pcm_close(playback.handle);  
    return 0;  
  
Err:  
    close(fd);  
    if (playback.data_buf) free(playback.data_buf);  
    if (playback.log) snd_output_close(playback.log);  
    if (playback.handle) snd_pcm_close(playback.handle);  
    return -1;  
}  
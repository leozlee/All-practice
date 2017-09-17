//File   : wav_parser.c  
//Author : Loon <sepnic@gmail.com>  
  
#include <assert.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <fcntl.h>  
  
#include "wav_parser.h"  
  
#define WAV_PRINT_MSG  
  
char* WAV_P_FmtString(uint16_t fmt)  
{  
    switch (fmt) {  
    case WAV_FMT_PCM:  
        return "PCM";  
        break;  
    case WAV_FMT_IEEE_FLOAT:  
        return "IEEE FLOAT";  
        break;  
    case WAV_FMT_DOLBY_AC3_SPDIF:  
        return "DOLBY AC3 SPDIF";  
        break;  
    case WAV_FMT_EXTENSIBLE:  
        return "EXTENSIBLE";  
        break;  
    default:  
        break;  
    }  
  
    return "NON Support Fmt";  
}  
  
void WAV_P_PrintHeader(WAVContainer_t *container)  
{  
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");  
    printf("\r\n");  
      
    printf("File Magic:         [%c%c%c%c]\r\n",   
        (char)(container->header.magic),   
        (char)(container->header.magic>>8),   
        (char)(container->header.magic>>16),   
        (char)(container->header.magic>>24));  
    printf("File Length:        [%d]\r\n", container->header.length);  
    printf("File Type:          [%c%c%c%c]\r\n",  
        (char)(container->header.type),   
        (char)(container->header.type>>8),   
        (char)(container->header.type>>16),   
        (char)(container->header.type>>24));  
          
    printf("\r\n");  
  
    printf("Fmt Magic:          [%c%c%c%c]\r\n",  
        (char)(container->format.magic),   
        (char)(container->format.magic>>8),   
        (char)(container->format.magic>>16),   
        (char)(container->format.magic>>24));  
    printf("Fmt Size:           [%d]\r\n", container->format.fmt_size);  
    printf("Fmt Format:         [%s]\r\n", WAV_P_FmtString(container->format.format));  
    printf("Fmt Channels:       [%d]\r\n", container->format.channels);  
    printf("Fmt Sample_rate:    [%d](HZ)\r\n", container->format.sample_rate);  
    printf("Fmt Bytes_p_second: [%d]\r\n", container->format.bytes_p_second);  
    printf("Fmt Blocks_align:   [%d]\r\n", container->format.blocks_align);  
    printf("Fmt Sample_length:  [%d]\r\n", container->format.sample_length);  
      
    printf("\r\n");  
  
    printf("Chunk Type:         [%c%c%c%c]\r\n",  
        (char)(container->chunk.type),   
        (char)(container->chunk.type>>8),   
        (char)(container->chunk.type>>16),   
        (char)(container->chunk.type>>24));  
    printf("Chunk Length:       [%d]\r\n", container->chunk.length);  
      
    printf("\r\n");  
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");  
}  
  
int WAV_P_CheckValid(WAVContainer_t *container)  
{  
    if (container->header.magic != WAV_RIFF ||  
        container->header.type != WAV_WAVE ||  
        container->format.magic != WAV_FMT ||  
        container->format.fmt_size != LE_INT(16) ||  
        (container->format.channels != LE_SHORT(1) && container->format.channels != LE_SHORT(2)) ||  
        container->chunk.type != WAV_DATA) {  
          
        fprintf(stderr, "non standard wav file.\r\n");  
        return -1;  
    }  
  
    return 0;  
}  
 //读取wav信息头及wav数据，保存到三个chunk上
int WAV_ReadHeader(int fd, WAVContainer_t *container)  
{  
    assert((fd >=0) && container);  
  
    if (read(fd, &container->header, sizeof(container->header)) != sizeof(container->header) ||   
        read(fd, &container->format, sizeof(container->format)) != sizeof(container->format) ||  
        read(fd, &container->chunk, sizeof(container->chunk)) != sizeof(container->chunk)) {  
  
        fprintf(stderr, "Error WAV_ReadHeader\r\n");  
        return -1;  
    }  
  
    if (WAV_P_CheckValid(container) < 0)  
        return -1;  
  
#ifdef WAV_PRINT_MSG  
    WAV_P_PrintHeader(container);  
#endif  
  
    return 0;  
}  
  
int WAV_WriteHeader(int fd, WAVContainer_t *container)  
{  
    assert((fd >=0) && container);  
      
    if (WAV_P_CheckValid(container) < 0)  
        return -1;  
  
    if (write(fd, &container->header, sizeof(container->header)) != sizeof(container->header) ||   
        write(fd, &container->format, sizeof(container->format)) != sizeof(container->format) ||  
        write(fd, &container->chunk, sizeof(container->chunk)) != sizeof(container->chunk)) {  
          
        fprintf(stderr, "Error WAV_WriteHeader\r\n");  
        return -1;  
    }  
  
#ifdef WAV_PRINT_MSG  
    WAV_P_PrintHeader(container);  
#endif  
  
    return 0;  
}  
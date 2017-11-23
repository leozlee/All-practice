//
// Created by leo on 11/17/17.
//

#include "sock.h"
#include <unistd.h>
#include <stdio.h>

struct timeval timeout={15,0};//3s

#define PIC_BUF_LEN 1124
static char pic_tmp[PIC_BUF_LEN];


/******
名  称:socket函数
参  数:domain指定通信域，type 指定通信类型，protocol 指定通信协议
返回值:成功:文件描述符;失败返回-1
******/
int Socket(int domain,int type,int protocol)
{
	int SockFd = -1;
	SockFd = socket(domain,type,protocol);
	if(-1 == SockFd)
	{
		perror("socket fail!!\r\n");
		return -1;
	}
	setsockopt(SockFd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
	setsockopt(SockFd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));


	return SockFd;

}


/******
名  称:设置服务器地址端口
参  数:addr是server的地址信息结构体，ip+port
返回值:成功:文件描述符;失败返回-1
******/
void Set_Server(struct sockaddr_in *addr,char *ip,char *port)
{
	addr->sin_family = AF_INET;
	inet_pton(AF_INET,ip,&(addr->sin_addr));
	addr->sin_port = htons(atoi(port));
}


/******
名  称:connet函数，用于连接服务端
参  数:sockfd是文件描述符，addr是一个包含服务端地址信息结构体的指针，addrlen地址长度
返回值:成功:文件描述符;失败返回-1
******/
int Connet(int sockfd,struct sockaddr_in *addr,socklen_t addrlen)
{
	int ret = -1;
	ret = connect(sockfd,(struct sockaddr *)addr,addrlen);
	if(-1 == ret)
	{
		perror("connect fail");
		return -1;
	}
	return 0;
}


/******
名  称:写函数
参  数:sockfd是文件描述符，buf待写内容，size待写字节数
返回值:成功:成功写入的字节数;失败返回-1
******/
int SockWrite(int sockfd,char *buf,int size)
{
	int ret = -1;
	ret = write(sockfd,buf,size);
	if(ret < 0)
	{
		perror("sock write fail\r\n");
		return -1;
	}
	return ret;
}


/******
名  称:读函数
参  数:sockfd是文件描述符，buf待写内容，count待读字节数
返回值:成功:成功读取的字节数;失败返回-1
******/
int SockRead(int sockfd,char *buf,int count)
{
	int ret = -1;
	//ret = recv(sockfd,buf,count,0);
	ret = read(sockfd,buf,count);
	if(-1 == ret)
	{
		perror("read fail");
		return -1;
	}

	return ret;
}


void CloseSock(int fd)
{
	close(fd);
}



int SendPic2Server(int fd, char *path)
{

	if(fd <0 || path ==NULL )
	{
		return -1;
	}

	int Filesize     = 0;
	FILE *PicFd     =NULL;
	char *P            = NULL;
	int ret              = 0;
	int blocknum = 0;
	int modnum   = 0;
	int j                  = 0;
	int cnt             = 0;
	char *current =NULL;
	char *tmp = NULL;

	FILE *G = fopen("dd.txt","wb+");


	tmp = malloc(1024);
	if(NULL == tmp)
	{
		printf("malloc tmp fail\r\n");
		return -1;
	}

	PicFd = fopen(path,"r");
	if(NULL == PicFd)
	{
		perror("open file fail");
		return -1;
	}
	else {
		//just get file size first
		fseek(PicFd, 0L, SEEK_END);
		Filesize = ftell(PicFd);
		printf("Filesize is %d\r\n", Filesize);
		fseek(PicFd, 0L, SEEK_SET);

		P = (char *) malloc(Filesize);
		if (NULL == P) {
			printf("malloc fail\r\n");
			fclose(PicFd);
			return -1;
		}

		ret = fread(P, 1, Filesize, PicFd);
		if (ret != Filesize) {
			printf("read fail\r\n");
			free(P);
			fclose(PicFd);
			return -1;
		}
		else
		{
			printf("%d bytes read \r\n",Filesize);
		}

		 blocknum = Filesize/PIC_BUF_LEN;
		 modnum   = Filesize%PIC_BUF_LEN;

		if(modnum > 0)
		{
			blocknum++;
			printf("T times:%d\r\n",blocknum);
		}

		for(j = 0; j < blocknum; j++)
		{
			current = P+ j*1024;
			bzero(tmp,1024);
			memmove(tmp,current,1024);
			sprintf(pic_tmp,"%s,%d,%d,%d,","*HQ,864811033098920,V8,1",blocknum,j,1024);
			memmove(pic_tmp+strlen(pic_tmp), P+ j*1024,1024);
			sprintf(pic_tmp+strlen(pic_tmp),"%s",",A,2233.1055,N,1138.1257,E,51.00,000,171121,102400,FFFFFFFF,#");
			ret = fwrite(pic_tmp,1,strlen(pic_tmp),G);
			fflush(G);
			//ret = SockWrite(fd,pic_tmp,strlen(pic_tmp));
			if(ret != strlen(pic_tmp))
			{
				perror("Something happen to write socket\r\n");
				break;
			}

			printf("block num:%d\r\n",j);
			cnt += ret;
			printf("cnt is %d\r\n",cnt);
			if(Filesize - cnt <1024)
			{
				current  = P + cnt;
				memmove(tmp,current,Filesize - cnt);
				sprintf(pic_tmp,"%s,%d,%d,%d,","*HQ,864811033098920,V8,1",blocknum,j+1,Filesize - cnt);
				memmove(pic_tmp+strlen(pic_tmp),  P + cnt,Filesize - cnt);
				sprintf(pic_tmp+strlen(pic_tmp),"%s",",A,2233.1055,N,1138.1257,E,51.00,000,171121,102400,FFFFFFFF,#");
				//sprintf(pic_tmp,"%s,%d,%d,%d,%s,%s","*HQ,864811033098920,V8,1",blocknum,j+1,1024,tmp,"A,2233.1055,N,1138.1257,E,51.00,000,171121,102400,FFFFFFFF,#");
				printf("This is the last block:%d\r\n",j+1);
				//printf("%d bytes write\r\n",fwrite(pic_tmp,1,strlen(pic_tmp),G));

				//ret = SockWrite(fd,pic_tmp,strlen(pic_tmp));
				ret = fwrite(pic_tmp,1,strlen(pic_tmp),G);
				fflush(G);
				if(ret != strlen(pic_tmp))
				{
					perror("Something happen to write socket last block\r\n");
				}
				break;
			}
		}
		printf("transmission complete\r\n");

		fclose(PicFd);
		fclose(G);
		free(P);
		free(tmp);
		return 0;
	}
}

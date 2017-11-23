//
// Created by leo on 11/23/17.
//


#include <stdio.h>
#include <unistd.h>
#include "sock.h"

char *LOGIN="*HQ,864811033098920,V6,MA2017110710,+00.00,,,5,4,3,120,010,1125,,020,040,000,000,2,011604120001,192.211.54.145,50001,1,1,1,255,010,0,120,4,8,05,030,#";
char SockBuf[128];



#define FILE_NAME "test.jpeg"





int main(int argv,char **argc)
{
	printf("###This is a demo for uart of mdvr!####GOOD LUCK!!!\r\n");
	if(argv < 3)
	{
		printf("Usage :./From688B ip port\r\n");
		return -1;
	}

	int sockfd = -1;
	int ret = 0;
	struct sockaddr_in svr_addr = {0};
//
	sockfd = Socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)
	{
		perror("Socket");
		return -1;
	}

	Set_Server(&svr_addr,argc[1],argc[2]);
	//Set_Server(&svr_addr,SVR_IP,SVR_PORT);

	ret = Connet(sockfd,&svr_addr,sizeof(struct sockaddr_in));
	if(-1 == ret)
	{
		printf("Connect Error\r\n");
		close(sockfd);
		return -1;
	}
	else
	{
		//Here just login
		ret = SockWrite(sockfd,LOGIN,strlen(LOGIN));
		if(ret)
		{
			bzero(SockBuf,128);
			ret = SockRead(sockfd,SockBuf,16);
			//*HQ171120085432#
			if(SockBuf[0] == '*' && SockBuf[1] == 'H' && SockBuf[2] == 'Q')
			{
				printf("Login\r\n");
			}
			else
			{
				printf("Something happen to Login\r\n");
				close(sockfd);
				return -1;
			}
		}
	}

	while(1)
	{
		//TODO:to check CMD to capture different channel
		SendPic2Server(sockfd,FILE_NAME );
		break;
	}

	close(sockfd);

	return 0;

}

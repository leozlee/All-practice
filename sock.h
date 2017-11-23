#ifndef __SOCK_H__
#define __SOCK_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/******
名  称:设置服务器地址端口
参  数:addr是server的地址信息结构体，ip+port
返回值:成功:文件描述符;失败返回-1
******/
void Set_Server(struct sockaddr_in *addr,char *ip,char *port);


/******
名  称:socket函数
参  数:domain指定通信域，type 指定通信类型，protocol 指定通信协议
返回值:成功:文件描述符;失败返回-1
******/
int Socket(int domain,int type,int protocol);



/******
名  称:connet函数，用于连接服务端
参  数:sockfd是文件描述符，addr是一个包含服务端地址信息结构体的指针，addrlen地址长度
返回值:成功:文件描述符;失败返回-1
******/
int Connet(int sockfd,struct sockaddr_in *addr,socklen_t addrlen);


/******
名  称:写函数
参  数:sockfd是文件描述符，buf待写内容，size待写字节数
返回值:成功:成功写入的字节数;失败返回-1
******/
int SockWrite(int sockfd,char *buf,int size);


/******
名  称:读函数
参  数:sockfd是文件描述符，buf待写内容，count待读字节数
返回值:成功:成功读取的字节数;失败返回-1
******/
int SockRead(int sockfd,char *buf,int count);


void CloseSock(int fd);

int SendPic2Server(int fd, char *path);


#endif
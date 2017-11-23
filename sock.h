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
��  ��:���÷�������ַ�˿�
��  ��:addr��server�ĵ�ַ��Ϣ�ṹ�壬ip+port
����ֵ:�ɹ�:�ļ�������;ʧ�ܷ���-1
******/
void Set_Server(struct sockaddr_in *addr,char *ip,char *port);


/******
��  ��:socket����
��  ��:domainָ��ͨ����type ָ��ͨ�����ͣ�protocol ָ��ͨ��Э��
����ֵ:�ɹ�:�ļ�������;ʧ�ܷ���-1
******/
int Socket(int domain,int type,int protocol);



/******
��  ��:connet�������������ӷ����
��  ��:sockfd���ļ���������addr��һ����������˵�ַ��Ϣ�ṹ���ָ�룬addrlen��ַ����
����ֵ:�ɹ�:�ļ�������;ʧ�ܷ���-1
******/
int Connet(int sockfd,struct sockaddr_in *addr,socklen_t addrlen);


/******
��  ��:д����
��  ��:sockfd���ļ���������buf��д���ݣ�size��д�ֽ���
����ֵ:�ɹ�:�ɹ�д����ֽ���;ʧ�ܷ���-1
******/
int SockWrite(int sockfd,char *buf,int size);


/******
��  ��:������
��  ��:sockfd���ļ���������buf��д���ݣ�count�����ֽ���
����ֵ:�ɹ�:�ɹ���ȡ���ֽ���;ʧ�ܷ���-1
******/
int SockRead(int sockfd,char *buf,int count);


void CloseSock(int fd);

int SendPic2Server(int fd, char *path);


#endif
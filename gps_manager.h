#ifndef _GPS_MANAGER_H_
#define _GPS_MANAGER_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <termio.h>
#include <cstdio>
#include <iostream>
#include <stdlib.h>

#include "utils.h"
#include "gps_data.h"


#define		UART1			"/dev/ttyS1"



class CGpsManager
{
public:
	CGpsManager();
	~CGpsManager();
	bool Init(const char *uart_dev = (char *)UART1);
	void DaemonProcess();

	bool CheckRet(char* buf,int STEP);

	static CGpsManager* GetInstance()
	{
		if (!mInstance)
			mInstance = new CGpsManager();
		return mInstance;
	}
	void Delete() { delete this;  }

	bool Mc20SendData(const char* data);
	static int cbGetGpsMsg(void* ptrThis, char* ptrBuf, int nSize);
	int GetTime(tc::TimeStamp &time);
	void GetData(struct GpsData &);				// 获取GPS数据，获取上一次的gps数据
	int SetSpeed(int fd, int speed);
	int SetParity(int fd, int databits, int stopbits, int parity);
	bool CalcCheck(const char *data);
	float GetSpeed();
	bool IsAvailable(); 						// 是否存在GPS信号

	bool READ_TEST();

	int ConfigInit(void);

public:
	static CGpsManager* mInstance;
private:
	int				        mThreadStatus;	//线程状态 0:空闲 1:忙碌
	int				        mInitialized;
	int				        mGpsfd;
	int				        mPwrFd;			//驱动文件节点
	tc::TimeStamp	mTime;
	float			        mSpeed;
	GpsData		    mGpsData;
	bool 			        mGpsState;						//gps状态
	bool			        mConfigured;
};

#endif

#include "gps_manager.h"

const speed_t speed_arr[] = { B230400, B115200, B57600, B38400, B19200, B9600, B4800, };
const int name_arr[] = { 230400, 115200, 57600, 38400, 19200, 9600, 4800, };


//$GNRMC,014206.00,A,2314.34010,N,11316.83154,E,0.034,,110917,,,A,V*10

#define		    BUFSIZE	 	        100
#define 	        VERSION 	        1
#define 	        GPRMC_LEN 	80


//close all
//$CCRMO,,3,*4F\r\n
//open RMC only
//$CCRMO,RMC,2,1.0*3D\r\n



static char GpsReadBuf[BUFSIZE];//ａｔ指令缓冲区
static char GPRMC_Buf[GPRMC_LEN];//存放定位信息

static const char *uart1 = "/dev/ttyAMA2";

CGpsManager* CGpsManager::mInstance = NULL;

CGpsManager::CGpsManager()
{
	mInitialized	            = false;
	mGpsState		        = false;
	mConfigured		    = false;
	//mTime.available     = 0;
	mSpeed			            = -1.0;
	mGpsfd			            = -1;
	mPwrFd			            = -1;

	Init(uart1);
}

CGpsManager::~CGpsManager()
{

}


bool CGpsManager::READ_TEST()
{
	//先读取raw数据
	int len = 0;
	bzero(GpsReadBuf, BUFSIZE);
	std::cout << "read before" << std::endl;
	len = read(mGpsfd, GpsReadBuf, BUFSIZE);
	if(len < 0)
	{
		perror("read error");
	}
	else
	{
		std::string rawdata = GpsReadBuf;
		printf( "rawdata is %s", rawdata.c_str());
	}
};


bool CGpsManager::Init(const char *uart_dev)
{
	//open uart
	mGpsfd = open(uart_dev, O_RDWR | O_NDELAY);
	//mGpsfd = open(uart_dev, O_RDWR);
	if (mGpsfd < 0)
	{
		perror("something happen to open Gps");
		return false;
	}
	else
	{
		std::cout << mGpsfd << std::endl;
		//配置串口速率
		if(false == SetSpeed(mGpsfd, 9600))
		{
			close(mGpsfd);
			return false;
		}
		//配置串口其他参数
		if(false == SetParity(mGpsfd, 8, 1, 'n'))
		{
			close(mGpsfd);
			return false;
		}
		mInitialized = true;
		memset(&mGpsData, '\0', sizeof(mGpsData));
		write(mGpsfd,"$CCRMO,,3,*4F\r\n",15);
		sleep(2);
		write(mGpsfd,"$CCRMO,RMC,2,1.0*3D\r\n",21);
		return true;
	}
}
int CGpsManager::DeInit()
{
	close(mGpsfd);
}

void CGpsManager::DaemonProcess()
{
	if (mInitialized == false)
	{
		Init(uart1);
	}
	else
	{
		if (mGpsfd > 0)
		{
			//先读取raw数据
			std::cout << "start to deal with" << std::endl;

			int len = 0;
			bzero(GpsReadBuf, BUFSIZE);
			len = read(mGpsfd, GpsReadBuf, BUFSIZE);
			if (len < 0)//判断信息包的数据大小
			{
				mGpsState                = false;
				mTime.available      = 0;
				mGpsData.status     = 'V';
				printf("something happen to read gps raw data");
				return;
			}
			else
			{
				std::string rawdata = GpsReadBuf;
				printf( "rawdata is %s", rawdata.c_str());
				int S_pos = rawdata.find("GNRMC",0);
				if(std::string::npos == S_pos)//查找数据帧头是否正确
				{
					mGpsState = false;
					mTime.available = 0;
					mGpsData.status = 'V';
					printf( "this frame head is corrupt");
					memset(&mGpsData, '\0', sizeof(mGpsData));
					return;
				}
				int E_pos = rawdata.find('*',S_pos);
				if(std::string::npos == E_pos)//确定数据帧尾是否存在
				{
					mGpsState = false;
					mTime.available = 0;
					mGpsData.status = 'V';
					printf("this frame end is corrupt");
					memset(&mGpsData, '\0', sizeof(mGpsData));
					return;
				}

				//校验数据
				if(!CalcCheck(rawdata.c_str()))
				{
					mGpsData.status     = 'V';
					mGpsState                = false;
					mTime.available      = 0;
					printf("something happen to check gps data");
					memset(&mGpsData, '\0', sizeof(mGpsData));
					return ;
				}

				std::vector<std::string> VEC;
				Split(rawdata.c_str(),",",VEC);

				//检查数据是否有效
				//$GNRMC,040302.50,V,,,,,,,110917,,,N,V*16
				if('V' == rawdata.c_str()[18])
				{
					mGpsState                 = false;
					mGpsData.status     = 'V';
					mTime.available      =  0;
					printf("The location message is invaild");
					memset(&mGpsData, '\0', sizeof(mGpsData));
					return ;
				}

				//$GPRMC,104524.000,A,2314.3313,N,11316.8391,E,0.09,211.05,210617,,,A*64
				//$GNRMC,082639.000,A,2314.3731,N,11316.8367,E,1.19,184.06,040717,,,A*7D	Mc20
				//$GNRMC,023804.50,A,2314.34077,N,11316.83509,E,0.075,,110917,,,A,V*11
				//填充GPSData
				//这部分是固定位置的数据

				mGpsData.status 		            = 'V';					                                	// GPS数据是否有效，‘A’表示有效，‘V’表示
				mGpsData.speed				        = (int)((atof(&VEC[7].c_str()[0]) * 1.852) * 100);	// 速度值,9999 = 99.99公里/小时,1节 = 1.852公里
				mSpeed						                = ((atof(&VEC[7].c_str()[0]) * 1.852));

				mGpsData.latDirection  	  	= VEC[4].c_str()[0];						                            // 纬度的方向, 'N'表示北纬，'S'表示南纬
				mGpsData.latDegree 			= (atoi(&VEC[3].c_str()[0])/100);			                // 纬度值的度
				mGpsData.latCent         	        = (atoi(&VEC[3].c_str()[0]) % 100);			                // 纬度值的分
				mGpsData.latSec          	        = (int)((atof(&VEC[3].c_str()[0]) - atoi(&VEC[3].c_str()[0]))*60*10000000);	// 纬度值的秒

				mGpsData.lngDirection 		= VEC[6].c_str()[0];						                                // 经度的方向,'E'表示东经,'W'表示西经
				mGpsData.lngDegree 			= (atoi(&VEC[5].c_str()[0]) / 100);			                // 经度值的度
				mGpsData.lngCent 			    = (atoi(&VEC[5].c_str()[0]) % 100);		                    // 经度值的分
				mGpsData.lngSec				    = (int)((atof(&VEC[5].c_str()[0]) - atoi(&VEC[5].c_str()[0]))*60*10000000);  		// 经度值的秒

				mTime.available                     	= 1;
				mTime.hour 			                    = (atol(&VEC[1].c_str()[0]) / 10000);
				mTime.minute 		                    = (atol(&VEC[1].c_str()[0]) % 10000) / 100;
				mTime.second 		                    = (atol(&VEC[1].c_str()[0]) % 100);
				mTime.year 			                    = (atol(&VEC[9].c_str()[0]) % 100) + 2000;
				mTime.month 		                    = (atol(&VEC[9].c_str()[0]) % 10000) / 100;
				mTime.day 			                    = (atol(&VEC[9].c_str()[0]) / 10000);


#if  1
				std::cout << "test for deal gps data" << std::endl;
				std::cout <<" Gps speed is "<< mGpsData.speed << std::endl;
				std::cout <<" Gps status is "<< mGpsData.status << std::endl;
				std::cout <<"what i get"<<mGpsData.latDegree ;
				printf("lat is%d.%d.%d\r\n",mGpsData.latDegree ,mGpsData.latCent ,mGpsData.latSec );
				printf("lng is%d.%d.%d\r\n",mGpsData.lngDegree ,mGpsData.lngCent ,mGpsData.lngSec );
				std::cout << "Today is "  << mTime.year << "/" << mTime.month << "/" << mTime.day << std::endl;
				std::cout <<"TIme " << mTime.hour << ":" << mTime.minute << ":" << mTime.second <<std::endl;
#endif

				mGpsState = true;

			}
		}
	}

}



float CGpsManager::GetSpeed()
{
	float speed = mSpeed;

	if(mGpsState)
		speed = mSpeed;
	else
		speed  = 0.0;

	return speed;
}

bool CGpsManager::IsAvailable()
{
	return mGpsState;
}
/**
*@function: set serial speed
*@param:fd
*@retval:data transfer speed
*/
int CGpsManager::SetSpeed(int fd, int speed)
{
	unsigned int i;
	struct termios Opt;    //定义termios结构

	if (tcgetattr(fd, &Opt) != 0)
	{
		perror("tcgetattr fd");
		return false;
	}
	for (i = 0; i < sizeof(speed_arr) / sizeof(speed_t); i++)
	{
		if (speed == name_arr[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			if (tcsetattr(fd, TCSANOW, &Opt) != 0)
			{
				perror("tcsetattr fd");
				return false;
			}
			tcflush(fd, TCIOFLUSH);
		}
	}
	return true;
}

int CGpsManager::GetTime(tc::TimeStamp &time)
{

	int ret = -1;
	time.available	= mTime.available;
	if (mTime.available)
	{
		time.year		    = mTime.year;
		time.month		= mTime.month;
		time.day		    = mTime.day;
		time.hour		    = mTime.hour;
		time.minute		= mTime.minute;
		time.second		= mTime.second;
		ret = 0;
	}

	return ret;
}

/** 
*@function: open serial device
*@param:void
*@retval:void
*/
int CGpsManager::SetParity(int fd, int databits, int stopbits, int parity)
{
	struct termios Opt;
	if (tcgetattr(fd, &Opt) != 0)
	{
		perror("tcgetattr fd");
		return false;
	}
	Opt.c_cflag |= (CLOCAL | CREAD);        //一般必设置的标志

	switch (databits)        //设置数据位数
	{
		case 7:
			Opt.c_cflag &= ~CSIZE;
			Opt.c_cflag |= CS7;
			break;
		case 8:
			Opt.c_cflag &= ~CSIZE;
			Opt.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr, "Unsupported data size.\n");
			return false;
	}

	switch (parity)            //设置校验位
	{
		case 'n':
		case 'N':
			Opt.c_cflag &= ~PARENB;        //清除校验位
			Opt.c_iflag &= ~INPCK;        //enable parity checking
			break;
		case 'o':
		case 'O':
			Opt.c_cflag |= PARENB;        //enable parity
			Opt.c_cflag |= PARODD;        //奇校验
			Opt.c_iflag |= INPCK;         //disable parity checking
			break;
		case 'e':

		case 'E':
			Opt.c_cflag |= PARENB;         //enable parity
			Opt.c_cflag &= ~PARODD;        //偶校验
			Opt.c_iflag |= INPCK;          //disable pairty checking
			break;

		case 's':

		case 'S':
			Opt.c_cflag &= ~PARENB;        //清除校验位
			Opt.c_cflag &= ~CSTOPB;        //??????????????
			Opt.c_iflag |= INPCK;          //disable pairty checking
			break;

		default:
			fprintf(stderr, "Unsupported parity.\n");
			return false;
	}

	switch (stopbits)        //设置停止位
	{
		case 1:
			Opt.c_cflag &= ~CSTOPB;
			break;
		case 2:
			Opt.c_cflag |= CSTOPB;
			break;

		default:
			fprintf(stderr, "Unsupported stopbits.\n");

			return false;
	}

	//Opt.c_cflag &= ~CRTSCTS; // 设置无RTS/CTS硬件流控
	Opt.c_cflag |= (CLOCAL | CREAD);
	Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);         //cancle echo
	Opt.c_lflag |= ICANON;        //标准接收，回车返回
	//Opt.c_lflag &= ~ICANON;      //非标准，能接收‘OA’数据(/n)
	Opt.c_oflag &= ~OPOST;
	Opt.c_oflag &= ~(ONLCR | OCRNL);    //添加的
	Opt.c_iflag &= ~(ICRNL | INLCR);
	Opt.c_iflag &= ~(IXON | IXOFF | IXANY);    //添加的
	tcflush(fd, TCIFLUSH);
	Opt.c_cc[VTIME] = 2;        //设置超时为15sec
	Opt.c_cc[VMIN] = 20;        //等待读取的最小字节数
	if (tcsetattr(fd, TCSANOW, &Opt) != 0)//Update the Opt and do it now
	{
		perror("tcsetattr fd");
		return false;
	}
	return true;
}

/*
*@功能: 校验定位数据是否正确（异或校验算法）
*@参数 data:校验的数据
*@返回值: true:校验成功 false:校验失败
*/
bool CGpsManager::CalcCheck(const char *data)
{
	int i, result;
	int temp = 0;
	for (result = data[1], i = 2; data[i] != '*'; i++)
	{
		result ^= data[i];
	}

	//return result;

	i++;
	if (data[i] >= '0' && data[i] <= '9')
	{
		temp = data[i] - 0x30;
	}
	else if (data[i] >= 'A' && data[i] <= 'F')
	{
		temp = data[i] - 0x57;
	}
	else
	{
		temp = 0;
	}
	temp = temp * 16;
	i++;
	if (data[i] >= '0' && data[i] <= '9')
	{
		temp |= ((data[i] - 0x30) & 0x0f);
	}
	else if (data[i] >= 'A' && data[i] <= 'F')
	{
		temp |= ((data[i] - 0x57) & 0x0f);
	}
	else
	{
		temp += 0;
	}
	if (temp == result)
	{
		return true;
	}
	else
	{
		return false;
	}

}

void CGpsManager::GetData(struct GpsData & Gps)				// 获取GPS数据，获取上一次的gps数据
{
	if (mGpsState) {
		memcpy(&Gps, &mGpsData, sizeof(struct GpsData));
	}
	else
		memset(&Gps,'\0',sizeof(Gps));
}

//获取gps结构体内容
int CGpsManager::cbGetGpsMsg(void* ptrThis, char* ptrBuf, int nSize)
{
	int ret = -1;
	CGpsManager* pThis = (CGpsManager *)ptrThis;
	if (ptrBuf == NULL)
	{
		return -1;
	}
	else
	{
		memcpy(ptrBuf, &(pThis->mGpsData), sizeof(pThis->mGpsData));
		return 0;
	}
}

bool CGpsManager::CheckRet(char* buf,int STEP)
{
	char *ptr = NULL;
	ptr = strstr(buf, "OK");
	if (NULL == ptr)
	{
		printf("STEP %d NOT OK\r\n",STEP);
		return false;
	}
	printf("STEP %d OK\r\n",STEP);
	return true;
}


bool CGpsManager::Mc20SendData(const char* data)
{
	unsigned char i=0;
	int ret = -1;
	for(i=0; data[i]!='\r'; i++)
	{
		if(i>100)
			return false;
	}
	i++;
	ret = write(mGpsfd,data,i);
	if(i != ret)
		return false;
	else
		return true;
}
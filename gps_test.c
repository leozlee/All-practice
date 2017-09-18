//
// Created by leo on 9/8/17.
//
//北斗模块定位信息解析程序，仅解析GPRMC信息（包含经纬度，海拔，速率，UTC时间）
//2017.9.1

#include <stdlib.h>
#include <stdio.h>
#define  u8 unsigned char
#define  u16 unsigned short int
#define  u32 unsigned int


//串口相关的头文件
#include<unistd.h>     /*Unix 标准函数定义*/
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>      /*文件控制定义*/
#include<termios.h>    /*PPSIX 终端控制定义*/
#include<errno.h>      /*错误号定义*/
#include<string.h>
//宏定义
#define FALSE  -1
#define TRUE   0

//自定义结构体
//GPS NMEA-0183协议重要参数结构体定义
//卫星信息
typedef struct
{
	u8 num;		//卫星编号
	u8 eledeg;	//卫星仰角
	u16 azideg;	//卫星方位角
	u8 sn;		//信噪比
}nmea_slmsg;
//UTC时间信息
typedef struct
{
	u16 year;	//年份
	u8 month;	//月份
	u8 date;	//日期
	u8 hour; 	//小时
	u8 min; 	//分钟
	u8 sec; 	//秒钟
}nmea_utc_time;
//NMEA 0183 协议解析后数据存放结构体
typedef struct
{
	u8 svnum;					//可见卫星数
	nmea_slmsg slmsg[12];		//最多12颗卫星
	nmea_utc_time utc;			//UTC时间
	u32 latitude;				//纬度 分扩大100000倍,实际要除以100000
	u8 nshemi;					//北纬/南纬,N:北纬;S:南纬
	u32 longitude;			    //经度 分扩大100000倍,实际要除以100000
	u8 ewhemi;					//东经/西经,E:东经;W:西经
	u8 gpssta;					//GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.
	u8 posslnum;				//用于定位的卫星数,0~12.
	u8 possl[12];				//用于定位的卫星编号
	u8 fixmode;					//定位类型:1,没有定位;2,2D定位;3,3D定位
	u16 pdop;					//位置精度因子 0~500,对应实际值0~50.0
	u16 hdop;					//水平精度因子 0~500,对应实际值0~50.0
	u16 vdop;					//垂直精度因子 0~500,对应实际值0~50.0

	int altitude;			 	//海拔高度,放大了10倍,实际除以10.单位:0.1m
	u16 speed;					//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时
}nmea_msg;


//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//       0XFF,代表不存在第cx个逗号
u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{
	u8 *p=buf;
	while(cx)
	{
		if(*buf=='*'||*buf<' '||*buf>'z')
			return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
		//*是每条信息的结尾
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;
}
//m^n函数
//返回值:m^n次方.
u32 NMEA_Pow(u8 m,u8 n)
{
	u32 result=1;
	while(n--)result*=m;
	return result;
}
//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
int NMEA_Str2num(u8 *buf,u8*dx)
{
	u8 *p=buf;
	u32 ires=0,fres=0;
	u8 ilen=0,flen=0,i;
	u8 mask=0;
	int res;
	while(1) //得到整数和小数的长度
	{
		if(*p=='-'){mask|=0X02;p++;}//是负数
		if(*p==','||(*p=='*'))break;//遇到结束了
		if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
		else if(*p>'9'||(*p<'0'))	//有非法字符
		{
			ilen=0;
			flen=0;
			break;
		}
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//去掉负号
	for(i=0;i<ilen;i++)	//得到整数部分数据
	{
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//最多取5位小数
	*dx=flen;	 		//小数点位数
	for(i=0;i<flen;i++)	//得到小数部分数据
	{
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	}
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;
	return res;
}


//分析GPRMC信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
//GNRMC信息示例：$GNRMC,033036.00,A,2314.33101,N,11316.83693,E,0.021,,010917,,,D,V*1E
//含义：                hhmmss
//buf为从北斗接收的字符串
void NMEA_GPRMC_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;
	u8 posx;
	u32 temp;
	float rs;
	p1=(u8*)strstr((const char *)buf,"GNRMC");//得到GNRMC在buf字符串中的位置
	posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间（时分秒）
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
		gpsx->utc.hour=temp/10000;
		gpsx->utc.min=(temp/100)%100;
		gpsx->utc.sec=temp%100;
	}
	posx=NMEA_Comma_Pos(p1,3);								//得到纬度
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'
		gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为°
	}
	posx=NMEA_Comma_Pos(p1,4);								//南纬还是北纬
	if(posx!=0XFF)gpsx->nshemi=*(p1+posx);
	posx=NMEA_Comma_Pos(p1,5);								//得到经度
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);
		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'
		gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为°
	}
	posx=NMEA_Comma_Pos(p1,6);								//东经还是西经
	if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);
	posx=NMEA_Comma_Pos(p1,9);								//得到UTC日期
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//得到UTC日期
		gpsx->utc.date=temp/10000;
		gpsx->utc.month=(temp/100)%100;
		gpsx->utc.year=2000+temp%100;
	}

	posx=NMEA_Comma_Pos(p1,7);								//得到速率
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);
		gpsx->speed=temp;	 	//单位： 公里/小时
	}
}

//提取NMEA-0183信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void GPS_Analysis(nmea_msg *gpsx,u8 *buf)
{
	NMEA_GPRMC_Analysis(gpsx,buf);	//GPRMC解析
}

//GPS解析部分到此结束，下面为串口部分

/*******************************************************************
* 名称：                  UART0_Open
* 功能：                 打开串口并返回串口设备文件描述
* 入口参数：             fd    :文件描述符     port :串口号(ttySMA1)
* 出口参数：            正确返回为1，错误返回为0
*******************************************************************/
int UART0_Open(int fd,char* port)
{

	fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
	if (FALSE == fd)
	{
		perror("Can't Open BD Serial Port");
		return(FALSE);
	}
	//恢复串口为阻塞状态
	if(fcntl(fd, F_SETFL, 0) < 0)
	{
		printf("fcntl failed!\n");
		return(FALSE);
	}
	else
	{
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
	}
	//测试是否为终端设备
	if(0 == isatty(STDIN_FILENO))
	{
		printf("standard input is not a terminal device\n");
		return(FALSE);
	}
	else
	{
		printf("isatty success!\n");
	}
	printf("fd->open=%d\n",fd);
	return fd;
}
/*******************************************************************
* 名称：                UART0_Close
* 功能：                关闭串口并返回串口设备文件描述
* 入口参数：            fd    :文件描述符
* 出口参数：            void
*******************************************************************/

void UART0_Close(int fd)
{
	close(fd);
}

/*******************************************************************
* 名称：                UART0_Set
* 功能：                设置串口数据位，停止位和效验位
* 入口参数：            fd   串口文件描述符
*                       speed     串口速度
*                       flow_ctrl   数据流控制
*                       databits   数据位   取值为 7 或者8
*                       stopbits   停止位   取值为 1 或者2
*                       parity     效验类型 取值为N,E,O,,S
*出口参数：            正确返回为1，错误返回为0
*******************************************************************/
int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{

	int   i;
	int   status;
	int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
	int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};

	struct termios options;

	/*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
	*/
	if  ( tcgetattr( fd,&options)  !=  0)
	{
		perror("Setup Serial faile!");
		return(FALSE);
	}

	//设置串口输入波特率和输出波特率
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{
		if  (speed == name_arr[i])
		{
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
			break;
		}
	}

	//修改控制模式，保证程序不会占用串口
	options.c_cflag |= CLOCAL;
	//修改控制模式，使得能够从串口中读取输入数据
	options.c_cflag |= CREAD;

	//设置数据流控制
	switch(flow_ctrl)
	{

		case 0 ://不使用流控制
			options.c_cflag &= ~CRTSCTS;
			break;

		case 1 ://使用硬件流控制
			options.c_cflag |= CRTSCTS;
			break;
		case 2 ://使用软件流控制
			options.c_cflag |= IXON | IXOFF | IXANY;
			break;
	}
	//设置数据位
	//屏蔽其他标志位
	options.c_cflag &= ~CSIZE;
	switch (databits)
	{
		case 5:
			options.c_cflag |= CS5;
			break;
		case 6:
			options.c_cflag |= CS6;
			break;
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr,"Unsupported data size\n");
			return (FALSE);
	}
	//设置校验位
	switch (parity)
	{
		case 'n':
		case 'N': //无奇偶校验位。
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O'://设置为奇校验
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
		case 'e':
		case 'E'://设置为偶校验
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;
		case 's':
		case 'S': //设置为空格
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
	}
	// 设置停止位
	switch (stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB; break;
		case 2:
			options.c_cflag |= CSTOPB; break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
			return (FALSE);
	}

	//修改输出模式，原始数据输出
	options.c_oflag &= ~OPOST;

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//我加的
	//options.c_lflag &= ~(ISIG | ICANON);

	//设置等待时间和最小接收字符
	//options.c_cc[VTIME] = 0; /* 读取一个字符等待1*(1/10)s */
	options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */

	//如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
	tcflush(fd,TCIFLUSH);

	//激活配置 (将修改后的termios数据设置到串口中）
	if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		perror("com set error!\n");
		return (FALSE);
	}
	return (TRUE);
}
/*******************************************************************
* 名称：                UART0_Init()
* 功能：                串口初始化
* 入口参数：            fd       :  文件描述符
*                       speed  :  串口速度
*                       flow_ctrl  数据流控制
*                       databits   数据位   取值为 7 或者8
*                       stopbits   停止位   取值为 1 或者2
*                       parity     效验类型 取值为N,E,O,,S
*
* 出口参数：        正确返回为1，错误返回为0
*******************************************************************/
int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
	int err;
	//设置串口数据帧格式
	if (UART0_Set(fd,9600,0,8,1,'N') == FALSE)
	{
		return FALSE;
	}
	return  TRUE;

}

/*******************************************************************
* 名称：                  UART0_Recv
* 功能：                  接收串口数据
* 入口参数：              fd          :文件描述符
*                         rcv_buf     :接收串口中数据存入rcv_buf缓冲区中
*                         data_len    :一帧数据的长度
* 出口参数：              正确返回为1，错误返回为0
*******************************************************************/
int UART0_Recv(int fd, char *rcv_buf,int data_len)
{
	int len,fs_sel;
	fd_set fs_read;

	struct timeval time;

	FD_ZERO(&fs_read);
	FD_SET(fd,&fs_read);

	time.tv_sec = 10;
	time.tv_usec = 0;

	//使用select实现串口的多路通信
	fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
	if(fs_sel)
	{
		len = read(fd,rcv_buf,data_len);
		printf("receive len = %d fs_sel = %d\n",len,fs_sel);
		return len;
	}
	else
	{
		printf("read fail!\n");
		return FALSE;
	}
}
/********************************************************************
* 名称：                UART0_Send
* 功能：                发送数据
* 入口参数：            fd           :文件描述符
*                       send_buf    :存放串口发送数据
*                       data_len    :一帧数据的个数
* 出口参数：            正确返回为1，错误返回为0
*******************************************************************/
int UART0_Send(int fd, char *send_buf,int data_len)
{
	int len = 0;

	len = write(fd,send_buf,data_len);
	if (len == data_len )
	{
		return len;
	}
	else
	{

		tcflush(fd,TCOFLUSH);
		return FALSE;
	}

}




int main()
{
	//手工赋值测试
	char buf[100] = "$GNRMC,041701.00,A,2314.33026,N,11316.83643,E,0.009,,010917,,,D,V*1B";
	nmea_msg gps_msg;
	GPS_Analysis(&gps_msg,buf);

	printf("latitude:%f %c\n",(float)gps_msg.latitude/100000 \
	,gps_msg.nshemi);
	printf("longitude:%f %c\n",(float)gps_msg.longitude/100000,gps_msg.ewhemi);
	printf("date:%4d-%02d-%02d\n",gps_msg.utc.year,gps_msg.utc.month,gps_msg.utc.date);
	printf("time:%02d:%02d:%02d\n",gps_msg.utc.hour,gps_msg.utc.min,gps_msg.utc.sec);


	//串口测试
	int fd;                            //文件描述符
	int err;                           //返回调用函数的状态
	int len;
	int i;
	char rcv_buf[2000];

	fd = UART0_Open(fd,"/dev/ttyAMA2"); //打开GPS串口，返回文件描述符
	do{
		err = UART0_Init(fd,9600,0,8,1,'N');
		printf("Set Port Exactly!\n");
	}while(FALSE == err || FALSE == fd);

	while (1) //循环读取数据
	{

		sleep(2);
		//由于其他消息没有关，所以消息很多，应将消息关闭，只保留GNRMC，
		//下面的数组就可以改小
		len = UART0_Recv(fd, rcv_buf,1500);
		tcflush(fd,TCOFLUSH); //清掉剩下未读的数据
		if(len > 0)
		{
			rcv_buf[len] = '\0';

			//这里一次打印太多系统会奔溃，所以先注释掉。
			//printf("receive data is %s\n",rcv_buf);
			printf("len = %d\n",len);

			GPS_Analysis(&gps_msg,rcv_buf);

			//打印读取到的数据，时间是北京时间
			printf("latitude:%f %c\n",(float)gps_msg.latitude/100000 \
			,gps_msg.nshemi);
			printf("longitude:%f %c\n",(float)gps_msg.longitude/100000,gps_msg.ewhemi);
			printf("date:%4d-%02d-%02d\n",gps_msg.utc.year,gps_msg.utc.month,gps_msg.utc.date);
			printf("time:%02d:%02d:%02d\n",gps_msg.utc.hour+8,gps_msg.utc.min,gps_msg.utc.sec);
			//printf("time:%04d\n",gps_msg.speed/1000);//这个速度的单位是啥还要具体看手册
			//速度还需要除以一定倍数
		}
		else
		{
			printf("cannot receive data\n");
		}

	}
	UART0_Close(fd);

	return 0;
}

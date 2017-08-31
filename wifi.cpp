#include <boost/include/boost/regex.hpp>
#include "wifi.h"
#include "utils/utils.h"
#include "log/logger.h"




CWifi* CWifi::sInstance = NULL;

#define     WIFI_PATH       "/proc/net/dev"
#define     MAC_SIZE        18
#define     TAG                 "CWIFI"
#define     BUF                 800

char  buf[BUF];

CWifi &CWifi::GetInstance() {
	if (!sInstance)
		sInstance = new CWifi();
	return *sInstance;
}

CWifi::CWifi() {
	//初始化变量为空
	mWifiAvailable = false;
	Getdata();
}

CWifi::~CWifi() {
	delete sInstance;
}

bool  CWifi::Getdata()
{
	//首先判断是否有wlan0网卡
	int fd = -1;
	fd = open(WIFI_PATH,O_RDONLY);
	if(fd < 0)
	{
		LOGD(TAG,"open wifi_pat");
		mWifiAvailable = false;
		return mWifiAvailable;
	}
	bzero(buf,800);
	if(read(fd,buf,800) < 0)
	{
		LOGD(TAG,"read error");
		close(fd);
		mWifiAvailable = false;
		return mWifiAvailable;
	}
	std::string rawdata = buf;
	int n =rawdata.find("wlan");        //here we will find the dev like wlan*
	if(std::string::npos == n)
	{
		LOGD(TAG,"dev not found");
	}
	else
	{
		mWifiAvailable = true;
		mdev = rawdata.substr(n,5);     //now we get the dev
	}
	close(fd);


	//next we will get info lifo like ip/mac ......

	int sd;
	struct sockaddr_in sin;
	struct ifreq ifr;

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sd)
	{
		LOGD(TAG,"socket error");
		mWifiAvailable = false;
		return mWifiAvailable;
	}

	strncpy(ifr.ifr_name, mdev.c_str(), IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;


	// if error: No such device
	if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
	{
		LOGD(TAG,"ioctl SIOCGIFADDR error");
		close(sd);
		mWifiAvailable = false;
		return mWifiAvailable;
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	mAddress=inet_ntoa(sin.sin_addr);               //ip get



	if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
	{
		LOGD(TAG,"ioctl  SIOCGIFHWADDR error");
		close(sd);
		mWifiAvailable = false;
		return mWifiAvailable;
	}
	else
	{
		bzero(buf,BUF);
		snprintf(buf, MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
		         (unsigned char)ifr.ifr_hwaddr.sa_data[0],
		         (unsigned char)ifr.ifr_hwaddr.sa_data[1],
		         (unsigned char)ifr.ifr_hwaddr.sa_data[2],
		         (unsigned char)ifr.ifr_hwaddr.sa_data[3],
		         (unsigned char)ifr.ifr_hwaddr.sa_data[4],
		         (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

		mMacAddress = buf;                              // mac get
	}


	if (ioctl(sd, SIOCGIFNETMASK, &ifr) < 0) {
		LOGD(TAG, "ioctl  SIOCGIFNETMASK error");
		close(sd);
		mWifiAvailable = false;
		return mWifiAvailable;
	}
	else
	{
		bzero(buf,BUF);
		snprintf(buf, MAC_SIZE, "%d.%d.%d.%d",
		         (unsigned char)ifr.ifr_netmask.sa_data[2],
		         (unsigned char)ifr.ifr_netmask.sa_data[3],
		         (unsigned char)ifr.ifr_netmask.sa_data[4],
		         (unsigned char)ifr.ifr_netmask.sa_data[5]);
		mMaskAddress = buf;
	}



	if (ioctl(sd, SIOCGIFBRDADDR, &ifr) < 0) {
		LOGD(TAG, "ioctl  SIOCGIFBRDADDR error");
		close(sd);
		mWifiAvailable = false;
		return mWifiAvailable;
	}
	else
	{
		bzero(buf,BUF);
		snprintf(buf, MAC_SIZE, "%d.%d.%d.%d",
		         (unsigned char)ifr.ifr_broadaddr.sa_data[2],
		         (unsigned char)ifr.ifr_broadaddr.sa_data[3],
		         (unsigned char)ifr.ifr_broadaddr.sa_data[4],
		         (unsigned char)ifr.ifr_broadaddr.sa_data[5]);
		mGatewayAddress = buf;
	}

	close(sd);

	mWifiAvailable = false;
	return mWifiAvailable;

}

bool CWifi::IsAvailable() {
	if(!mWifiAvailable)
		Getdata();
	return mWifiAvailable;
}

bool CWifi::GetApSSID(std::string &ssid) {
	ssid.clear();
	if(!Popen("iwgetid -r", ssid))
		return false;
	return true;
}

bool CWifi::GetCardAddress(std::string &addr) {
	addr = mAddress;
	return mWifiAvailable;
}

bool CWifi::GetCardMacAddress(std::string &addr) {
	addr = mMacAddress;
	return mWifiAvailable;
}

bool CWifi::GetCardMaskAddress(std::string &addr) {
	addr = mMaskAddress;
	return mWifiAvailable;
}

bool CWifi::GetCardGatewayAddress(std::string &addr) {
	addr = mGatewayAddress;
	return mWifiAvailable;
}

bool CWifi::SetCardAddress(std::string &addr) {
	return false;
}

bool CWifi::SetCardMaskAddress(std::string &addr) {
	return false;
}

bool CWifi::SetCardMacAddress(std::string &addr) {
	return false;
}

bool CWifi::SetCardGateWayAddress(std::string &addr) {
	return false;
}




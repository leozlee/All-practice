#include "wifi.h"


bool Popen(const char* cmd,std::string &result);

CWifi CWifi::sInstance;

CWifi &CWifi::GetInstance() {
	return sInstance;
}

CWifi::CWifi() {
	//初始化变量为空
	mWifiAvailable = false;

}

CWifi::~CWifi() {

}

void  CWifi::Getdata()
{
	//使用shell获取ifconfig wlan的信
	std::string result;
	if(!Popen("ifconfig | awk '/eth[0-9]/ {print $1}'", result))//查找是否wlan[0-9]用于判断是否有网卡
		mWifiAvailable = false;
	if(!result.empty())
		mWifiAvailable = true;

	if(!Popen(" ifconfig | awk '/eth[0-9]/ {print $1}' | xargs ifconfig", result))
		mWifiAvailable = false;

	//使用boost提取各类信息
	std::vector<boost::smatch> address;
	boost::smatch m;
	boost::regex reg_mac("\\w{2}:\\w{2}:\\w{2}:\\w{2}:\\w{2}:\\w{2}");
	boost::regex reg("\\d{1,3}.\\d{1,3}.\\d{1,3}.\\d{1,3}");

	//HWaddr
	boost::regex_search(result,m,reg_mac);
	mMacAddress = m[0];

	//利用boost_search遍历整个文本
	std::string::const_iterator st = result.begin();
	std::string::const_iterator end = result.end();
	while(boost::regex_search(st,end,m,reg))
	{
		st = m[0].second;
		address.push_back(m);
	}

	//信息保存在各自对应的变量当中
	mAddress            = address[0][0];
	mGatewayAddress     = address[1][0];
	mMaskAddress        = address[2][0];
}

bool CWifi::IsAvailable() {
	if(!mWifiAvailable)
		Getdata();
	return mWifiAvailable;
}

bool CWifi::GetApSSID(std::string &ssid) {
	return false;
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




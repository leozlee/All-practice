#include <boost/include/boost/regex.hpp>
#include "mobile_network.h"
#include "utils/utils.h"



CMobileNetwork CMobileNetwork::sInstance;

CMobileNetwork &CMobileNetwork::GetInstance() {
	return sInstance;
}

CMobileNetwork::CMobileNetwork() {
	//初始化变量为空
	mAvailable = false;

}

CMobileNetwork::~CMobileNetwork() {

}

void  CMobileNetwork::Getdata()
{
	//使用shell获取ifconfig eth的信息
	std::string result;
	if(!Popen("ifconfig | awk '/eth[0-9]/ {print $1}'", result))//查找是否eth[0-9]用于判断是否有网卡
		mAvailable = false;
	if(!result.empty())
		mAvailable = true;

	if(!Popen(" ifconfig | awk '/eth[0-9]/ {print $1}' | xargs ifconfig", result))
		mAvailable = false;

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
	mAddress                     = address[0][0];
	mGatewayAddress     = address[1][0];
	mMaskAddress           = address[2][0];
}

bool CMobileNetwork::IsAvailable() {
	if(!mAvailable)
		Getdata();
	return mAvailable;
}
//网络类型
unsigned int CMobileNetwork::GetNetworkType()
{
	return MOBILE_NETWORK_TYPE_3G_TD_WCDMA;
}


unsigned int  CMobileNetwork::GetNetworkQuality()
{
	return MOBILE_NETWORK_QUALITY_NORMAL;
}

bool CMobileNetwork::GetApSSID(std::string &ssid) {
	return false;
}

bool CMobileNetwork::GetCardAddress(std::string &addr) {
	addr = mAddress;
	return mAvailable;
}

bool CMobileNetwork::GetCardMacAddress(std::string &addr) {
	addr = mMacAddress;
	return mAvailable;
}

bool CMobileNetwork::GetCardMaskAddress(std::string &addr) {
	addr = mMaskAddress;
	return mAvailable;
}

bool CMobileNetwork::GetCardGatewayAddress(std::string &addr) {
	addr = mGatewayAddress;
	return mAvailable;
}

bool CMobileNetwork::SetCardAddress(std::string &addr) {
	return false;
}

bool CMobileNetwork::SetCardMaskAddress(std::string &addr) {
	return false;
}

bool CMobileNetwork::SetCardMacAddress(std::string &addr) {
	return false;
}

bool CMobileNetwork::SetCardGateWayAddress(std::string &addr) {
	return false;
}


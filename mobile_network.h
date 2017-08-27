//
// Created by root on 17-8-2.
//

#ifndef MOBILE_NETWORK_H
#define MOBILE_NETWORK_H



#include "common.h"
#include "utils/utils.h"
enum {
	MOBILE_NETWORK_UNKNOWN,
	MOBILE_NETWORK_TYPE_2G,
	MOBILE_NETWORK_TYPE_3G_EVD0,
	MOBILE_NETWORK_TYPE_3G_WCDMA,
	MOBILE_NETWORK_TYPE_3G_TD_WCDMA,
	MOBILE_NETWORK_TYPE_4G_TD_LED,
	MOBILE_NETWORK_TYPE_4G_FDD,
};

enum {
	MOBILE_NETWORK_QUALITY_UNKNOWN,
	MOBILE_NETWORK_QUALITY_NO_SIM,      // 没有SIM卡,无信号
	MOBILE_NETWORK_QUALITY_BAD,              // 信号差
	MOBILE_NETWORK_QUALITY_WEAK,        // 信号弱
	MOBILE_NETWORK_QUALITY_NORMAL,      // 信号一般
	MOBILE_NETWORK_QUALITY_STRONG,      // 信号强
};



class CMobileNetwork
{
public:
	static CMobileNetwork &GetInstance();

	// 是否可用
	bool IsAvailable();

	//网络类型
	unsigned int GetNetworkType();

	// 网络质量
	unsigned int GetNetworkQuality();

	// 获取AP的SSID
	bool GetApSSID(std::string &ssid);

	// 获取网卡地址,IPv4,不可用时返回false，否则返回true
	bool GetCardAddress(std::string &addr);

	// 获取网卡MAC地址,不可用时返回false，否则返回true
	bool GetCardMacAddress(std::string &addr);

	// 获取网卡子网掩码,IPv4,不可用时返回false，否则返回true
	bool GetCardMaskAddress(std::string &addr);

	// 获取网卡网关地址，IPv4,不可用时返回false，否则返回true
	bool GetCardGatewayAddress(std::string &addr);

	// 设置网卡地址,IPv4
	bool SetCardAddress(std::string &addr);

	// 设置网卡子网掩码,IPv4
	bool SetCardMaskAddress(std::string &addr);

	// 设置网卡MAC地址
	bool SetCardMacAddress(std::string &addr);

	// 设置网卡网关地址,IPv4
	bool SetCardGateWayAddress(std::string &addr);

private:
	static CMobileNetwork sInstance;

	bool           mAvailable;
	std::string mAddress;
	std::string mMacAddress;
	std::string mMaskAddress;
	std::string mGatewayAddress;

	CMobileNetwork();
	void Getdata(void);
	~CMobileNetwork();

	CMobileNetwork(const CMobileNetwork &);

	CMobileNetwork &operator=(const CMobileNetwork &);


};





#endif //MOBILE_NETWORK_H

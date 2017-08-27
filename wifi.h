#ifndef _WIFI_H_
#define _WIFI_H_

#include <iostream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

class CWifi {
public:
    static CWifi &GetInstance();

	// 是否可用
	bool IsAvailable();

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
    static CWifi sInstance;

	bool mWifiAvailable;
	std::string mAddress;
	std::string mMacAddress;
	std::string mMaskAddress;
	std::string mGatewayAddress;

    CWifi();
	void Getdata(void);
    ~CWifi();

	CWifi(const CWifi &);

	CWifi &operator=(const CWifi &);
};

#endif //_WIFI_H_

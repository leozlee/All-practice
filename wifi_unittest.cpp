//
// Created by leo on 8/20/17.
//

#include "wifi.h"
#include "gtest/gtest.h"

TEST(WIFI_UNITTEST, Isavailable) {
    EXPECT_TRUE(CWifi::GetInstance().IsAvailable());
}

TEST(WIFI_UNITTEST, GetCardAddress) {
    std::string addr;
    CWifi::GetInstance().GetCardAddress(addr);
    EXPECT_EQ("192.168.1.57", addr);
}

TEST(WIFI_UNITTEST, GetCardMacAddress) {
    std::string mac;
    CWifi::GetInstance().GetCardMacAddress(mac);
    EXPECT_EQ("00:0c:29:b6:d9:6a", mac);
}

TEST(WIFI_UNITTEST, GetCardMaskAddress) {
    std::string mac;
    CWifi::GetInstance().GetCardMaskAddress(mac);
    EXPECT_EQ("255.255.255.0", mac);
}

TEST(WIFI_UNITTEST, GetCardGatewayAddress) {
    std::string mac;
    CWifi::GetInstance().GetCardGatewayAddress(mac);
    EXPECT_EQ("192.168.1.255", mac);
}
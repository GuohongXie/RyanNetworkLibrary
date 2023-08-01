#include "net/inet_address.h"

#include "gtest/gtest.h"

#include "logger/logging.h"

class InetAddressTest : public ::testing::Test {
};

TEST_F(InetAddressTest, testInetAddress) {
  InetAddress addr0(0, "");
  EXPECT_EQ(addr0.ToIp(), std::string("0.0.0.0"));
  EXPECT_EQ(addr0.ToIpPort(), std::string("0.0.0.0:0"));
  EXPECT_EQ(addr0.ToPort(), 0);

  InetAddress addr1(8888);
  EXPECT_EQ(addr1.ToIp(), std::string("0.0.0.0"));
  EXPECT_EQ(addr1.ToIpPort(), std::string("0.0.0.0:8888"));
  EXPECT_EQ(addr1.ToPort(), 8888);

  InetAddress addr2(65535);
  EXPECT_EQ(addr2.ToIp(), std::string("0.0.0.0"));
  EXPECT_EQ(addr2.ToIpPort(), std::string("0.0.0.0:65535"));
  EXPECT_EQ(addr2.ToPort(), 65535);
}

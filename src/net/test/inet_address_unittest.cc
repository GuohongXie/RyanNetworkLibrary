#include "inet_address.h"

#include "logging.h"

//#define BOOST_TEST_MODULE InetAddressTest
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(testInetAddress) {
  InetAddress addr0(0, "");
  BOOST_CHECK_EQUAL(addr0.ToIp(), std::string("0.0.0.0"));
  BOOST_CHECK_EQUAL(addr0.ToIpPort(), std::string("0.0.0.0:0"));
  BOOST_CHECK_EQUAL(addr0.ToPort(), 0);

  InetAddress addr1(8888);
  BOOST_CHECK_EQUAL(addr1.ToIp(), std::string("0.0.0.0"));
  BOOST_CHECK_EQUAL(addr1.ToIpPort(), std::string("0.0.0.0:8888"));
  BOOST_CHECK_EQUAL(addr1.ToPort(), 8888);

  InetAddress addr2(65535);
  BOOST_CHECK_EQUAL(addr2.ToIp(), std::string("0.0.0.0"));
  BOOST_CHECK_EQUAL(addr2.ToIpPort(), std::string("0.0.0.0:65535"));
  BOOST_CHECK_EQUAL(addr2.ToPort(), 65535);
}

#ifndef RYANLIB_NET_INET_ADDRESS_H
#define RYANLIB_NET_INET_ADDRESS_H

#include <arpa/inet.h>
#include <netinet/in.h>

#include <cstring>
#include <string>

class InetAddress {
 public:
  explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
  explicit InetAddress(const sockaddr_in& addr) : addr_(addr) {}

  std::string ToIp() const;
  std::string ToIpPort() const;
  uint16_t ToPort() const;

  const sockaddr_in* GetSockAddr() const { return &addr_; }
  void SetSockAddr(const sockaddr_in& addr) { addr_ = addr; }

 private:
  sockaddr_in addr_;
};

#endif  // RYANLIB_NET_INET_ADDRESS_H
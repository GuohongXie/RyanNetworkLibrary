#ifndef RYANLIB_NET_SOCKET_H_
#define RYANLIB_NET_SOCKET_H_

#include "noncopyable.h"

class InetAddress;

// 封装socket fd
class Socket : Noncopyable {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket();

  // 获取sockfd
  int Fd() const { return sockfd_; }
  // 绑定sockfd
  void BindAddress(const InetAddress& localaddr);
  // 使sockfd为可接受连接状态
  void Listen();
  // 接受连接
  int Accept(InetAddress* peeraddr);

  // 设置半关闭
  void ShutdownWrite();

  void SetTcpNoDelay(bool on);  // 设置Nagel算法
  void SetReuseAddr(bool on);   // 设置地址复用
  void SetReusePort(bool on);   // 设置端口复用
  void SetKeepAlive(bool on);   // 设置长连接

 private:
  const int sockfd_;
};

#endif //RYANLIB_NET_SOCKET_H_
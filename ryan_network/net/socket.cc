#include "net/socket.h"

#include <cstring>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "net/inet_address.h"
#include "logger/logging.h"

Socket::~Socket() { ::close(sockfd_); }

void Socket::BindAddress(const InetAddress& localaddr) {
  if (0 != ::bind(sockfd_, (sockaddr*)localaddr.GetSockAddr(),
                  sizeof(sockaddr_in))) {
    LOG_FATAL << "bind sockfd:" << sockfd_ << " fail";
  }
}

void Socket::Listen() {
  if (0 != ::listen(sockfd_, 1024)) {
    LOG_FATAL << "listen sockfd:" << sockfd_ << " fail";
  }
}

int Socket::Accept(InetAddress* peeraddr) {
  /**
   * 1. accept函数的参数不合法
   * 2. 对返回的connfd没有设置非阻塞
   * Reactor模型 one loop per thread
   * poller + non-blocking IO
   **/
  sockaddr_in addr;
  socklen_t len = sizeof(addr);
  ::memset(&addr, 0, sizeof(addr));
  // fixed : int connfd = ::accept(sockfd_, (sockaddr *)&addr, &len);
  int connfd =
      ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  // int connfd = ::accept(sockfd_, (sockaddr *)&addr, &len);
  if (connfd >= 0) {
    peeraddr->SetSockAddr(addr);
  } else {
    LOG_ERROR << "accept4() failed";
  }
  return connfd;
}

// TODO:socket的各个设置的用法
void Socket::ShutdownWrite() {
  /**
   * 关闭写端，但是可以接受客户端数据
   */
  if (::shutdown(sockfd_, SHUT_WR) < 0) {
    LOG_ERROR << "shutdownWrite error";
  }
}

// 不启动Nagle算法
void Socket::SetTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval,
               sizeof(optval));  // TCP_NODELAY包含头文件 <netinet/tcp.h>
}

// 设置地址复用，其实就是可以使用处于Time-wait的端口
void Socket::SetReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval,
               sizeof(optval));  // TCP_NODELAY包含头文件 <netinet/tcp.h>
}

// 通过改变内核信息，多个进程可以绑定同一个地址。通俗就是多个服务的ip+port是一样
void Socket::SetReusePort(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval,
               sizeof(optval));  // TCP_NODELAY包含头文件 <netinet/tcp.h>
}

void Socket::SetKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
               sizeof(optval));  // TCP_NODELAY包含头文件 <netinet/tcp.h>
}
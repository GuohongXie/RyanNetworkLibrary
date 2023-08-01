#ifndef RYANLIB_NET_ACCEPTOR_H_
#define RYANLIB_NET_ACCEPTOR_H_

#include "net/channel.h"
#include "base/noncopyable.h"
#include "net/socket.h"

class EventLoop;
class InetAddress;

/**
 * Acceptor运行在mainLoop中
 * TcpServer发现Acceptor有一个新连接，则将此channel分发给一个subLoop
 */
class Acceptor {
 public:
  // 接受新连接的回调函数
  using NewConnectionCallback =
      std::function<void(int sockfd, const InetAddress&)>;
  Acceptor(EventLoop* loop, const InetAddress& ListenAddr, bool reuseport);
  ~Acceptor();

  void SetNewConnectionCallback(const NewConnectionCallback& cb) {
    new_connection_callback_ = cb;
  }

  bool listenning() const { return listenning_; }
  void Listen();

 private:
  void HandleRead();

  EventLoop* loop_;  // Acceptor用的就是用户定义的BaseLoop
  Socket accept_socket_;
  Channel accept_channel_;
  NewConnectionCallback new_connection_callback_;
  bool listenning_;  // 是否正在监听的标志
};

#endif  // RYANLIB_NET_ACCEPTOR_H_
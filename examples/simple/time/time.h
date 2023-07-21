#ifndef RYANLIB_EXAMPLES_SIMPLE_TIME_TIME_H_
#define RYANLIB_EXAMPLES_SIMPLE_TIME_TIME_H_

#include "tcp_server.h"

// RFC 868
class TimeServer {
 public:
  TimeServer(EventLoop* loop,
             const InetAddress& listen_addr);

  void Start();

 private:
  void OnConnection(const TcpConnectionPtr& conn);

  void OnMessage(const TcpConnectionPtr& conn,
                 Buffer* buf, muduo::Timestamp time);

  TcpServer server_;
};

#endif  // RYANLIB_EXAMPLES_SIMPLE_TIME_TIME_H_

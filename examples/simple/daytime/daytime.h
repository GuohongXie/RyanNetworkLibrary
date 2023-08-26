#ifndef RYANLIB_EXAMPLES_SIMPLE_DAYTIME_DAYTIME_H_
#define RYANLIB_EXAMPLES_SIMPLE_DAYTIME_DAYTIME_H_

#include "tcp_connection/tcp_server.h"

// RFC 867
class DaytimeServer {
 public:
  DaytimeServer(EventLoop* loop,
                const InetAddress& listenAddr);

  void Start();

 private:
  void OnConnection(const TcpConnectionPtr& conn);

  void OnMessage(const TcpConnectionPtr& conn,
                 Buffer* buf, Timestamp time);

  TcpServer server_;
};

#endif  // RYANLIB_EXAMPLES_SIMPLE_DAYTIME_DAYTIME_H_

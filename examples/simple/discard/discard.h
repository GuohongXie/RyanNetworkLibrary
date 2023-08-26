#ifndef RYANLIB_EXAMPLES_SIMPLE_DISCARD_DISCARD_H_
#define RYANLIB_EXAMPLES_SIMPLE_DISCARD_DISCARD_H_

#include "tcp_connection/tcp_server.h"

// RFC 863
class DiscardServer {
 public:
  DiscardServer(EventLoop* loop,
                const InetAddress& listenAddr);

  void Start();

 private:
  void OnConnection(const TcpConnectionPtr& conn);

  void OnMessage(const TcpConnectionPtr& conn,
                 Buffer* buf, Timestamp time);

  TcpServer server_;
};

#endif  // RYANLIB_EXAMPLES_SIMPLE_DISCARD_DISCARD_H_

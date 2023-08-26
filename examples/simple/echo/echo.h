#ifndef RYANLIB_EXAMPLES_SIMPLE_ECHO_ECHO_H_
#define RYANLIB_EXAMPLES_SIMPLE_ECHO_ECHO_H_

#include "tcp_connection/tcp_server.h"

// RFC 862
class EchoServer {
 public:
  EchoServer(EventLoop* loop,
             const InetAddress& listen_addr);

  void Start();  // calls server_.start();

 private:
  void OnConnection(const TcpConnectionPtr& conn);

  void OnMessage(const TcpConnectionPtr& conn,
                 Buffer* buf, Timestamp time);

  TcpServer server_;
};

#endif  // RYANLIB_EXAMPLES_SIMPLE_ECHO_ECHO_H_

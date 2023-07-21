#ifndef RYANLIB_EXAMPLES_SIMPLE_CHARGEN_CHARGEN_H_
#define RYANLIB_EXAMPLES_SIMPLE_CHARGEN_CHARGEN_H_

#include "tcp_server.h"

// RFC 864
class ChargenServer {
 public:
  ChargenServer(EventLoop* loop,
                const InetAddress& listen_addr, bool print = false);

  void Start();

 private:
  void OnConnection(const TcpConnectionPtr& conn);

  void OnMessage(const TcpConnectionPtr& conn,
                 Buffer* buf, Timestamp time);

  void OnWriteComplete(const TcpConnectionPtr& conn);
  void PrintThroughput();

  TcpServer server_;

  std::string message_;
  int64_t transferred_;
  Timestamp start_time_;
};

#endif  // RYANLIB_EXAMPLES_SIMPLE_CHARGEN_CHARGEN_H_

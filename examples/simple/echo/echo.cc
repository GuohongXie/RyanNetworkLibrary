#include "examples/simple/echo/echo.h"

#include "logger/logging.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


EchoServer::EchoServer(EventLoop* loop,
                       const InetAddress& listen_addr)
    : server_(loop, listen_addr, "EchoServer") {
  server_.SetConnectionCallback(std::bind(&EchoServer::OnConnection, this, _1));
  server_.SetMessageCallback(
      std::bind(&EchoServer::OnMessage, this, _1, _2, _3));
}

void EchoServer::Start() { server_.Start(); }

void EchoServer::OnConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "EchoServer - " << conn->PeerAddress().ToIpPort() << " -> "
           << conn->LocalAddress().ToIpPort() << " is "
           << (conn->Connected() ? "UP" : "DOWN");
}

void EchoServer::OnMessage(const TcpConnectionPtr& conn,
                           Buffer* buf, Timestamp time) {
  std::string msg(buf->RetrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
           << "data received at " << time.ToString();
  conn->Send(msg);
}

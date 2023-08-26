#include "examples/simple/discard/discard.h"
#include <functional>

#include "logger/logging.h"


DiscardServer::DiscardServer(EventLoop* loop, const InetAddress& listenAddr)
    : server_(loop, listenAddr, "DiscardServer") {
  server_.SetConnectionCallback(
      std::bind(&DiscardServer::OnConnection, this, std::placeholders::_1));
  server_.SetMessageCallback(
      std::bind(&DiscardServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void DiscardServer::Start() { server_.Start(); }

void DiscardServer::OnConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "DiscardServer - " << conn->PeerAddress().ToIpPort() << " -> "
           << conn->LocalAddress().ToIpPort() << " is "
           << (conn->Connected() ? "UP" : "DOWN");
}

void DiscardServer::OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
                              Timestamp time) {
  std::string msg(buf->RetrieveAllAsString());
  LOG_INFO << conn->name() << " discards " << msg.size()
           << " bytes received at " << time.ToString();
}

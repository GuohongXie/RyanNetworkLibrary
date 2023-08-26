#include "examples/simple/daytime/daytime.h"
#include <functional>

#include "logger/logging.h"
#include "net/event_loop.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

DaytimeServer::DaytimeServer(EventLoop* loop, const InetAddress& listenAddr)
    : server_(loop, listenAddr, "DaytimeServer") {
  server_.SetConnectionCallback(
      std::bind(&DaytimeServer::OnConnection, this, _1));
  server_.SetMessageCallback(
      std::bind(&DaytimeServer::OnMessage, this, _1, _2, _3));
}

void DaytimeServer::Start() { server_.Start(); }

void DaytimeServer::OnConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "DaytimeServer - " << conn->PeerAddress().ToIpPort() << " -> "
           << conn->LocalAddress().ToIpPort() << " is "
           << (conn->Connected() ? "UP" : "DOWN");
  if (conn->Connected()) {
    conn->Send(Timestamp::Now().ToFormattedString() + "\n");
    conn->Shutdown();
  }
}

void DaytimeServer::OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
                              Timestamp time) {
  std::string msg(buf->RetrieveAllAsString());
  LOG_INFO << conn->name() << " discards " << msg.size()
           << " bytes received at " << time.ToString();
}

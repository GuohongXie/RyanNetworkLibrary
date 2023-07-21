#include "time.h"

#include "logging.h"
#include "Endian.h"


TimeServer::TimeServer(EventLoop* loop,
                       const InetAddress& listen_addr)
    : server_(loop, listen_addr, "TimeServer") {
  server_.setConnectionCallback(std::bind(&TimeServer::OnConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&TimeServer::OnMessage, this, _1, _2, _3));
}

void TimeServer::Start() { server_.Start(); }

void TimeServer::OnConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "TimeServer - " << conn->PeerAddress().ToIpPort() << " -> "
           << conn->LocalAddress().ToIpPort() << " is "
           << (conn->Connected() ? "UP" : "DOWN");
  if (conn->Connected()) {
    time_t now = ::time(NULL);
    //大小端转换
    int32_t be32 = sockets::HostToNetwork32(static_cast<int32_t>(now));
    conn->Send(&be32, sizeof be32);
    conn->Shutdown();
  }
}

void TimeServer::OnMessage(const TcpConnectionPtr& conn,
                           Buffer* buf, Timestamp time) {
  std::string msg(buf->RetrieveAllAsString());
  LOG_INFO << conn->name() << " discards " << msg.size()
           << " bytes received at " << time.ToString();
}

#include "examples/simple/chargen/chargen.h"

#include <cstdio>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

ChargenServer::ChargenServer(EventLoop* loop, const InetAddress& listen_addr,
                             bool print)
    : server_(loop, listen_addr, "ChargenServer"),
      transferred_(0),
      start_time_(Timestamp::Now()) {
  server_.SetConnectionCallback(
      std::bind(&ChargenServer::OnConnection, this, _1));
  server_.SetMessageCallback(
      std::bind(&ChargenServer::OnMessage, this, _1, _2, _3));
  server_.SetWriteCompleteCallback(
      std::bind(&ChargenServer::OnWriteComplete, this, _1));
  if (print) {
    loop->RunEvery(3.0, std::bind(&ChargenServer::PrintThroughput, this));
  }

  std::string line;
  for (int i = 33; i < 127; ++i) {
    line.push_back(char(i));
  }
  line += line;

  for (size_t i = 0; i < 127 - 33; ++i) {
    message_ += line.substr(i, 72) + '\n';
  }
}

void ChargenServer::Start() { server_.Start(); }

void ChargenServer::OnConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "ChargenServer - " << conn->PeerAddress().ToIpPort() << " -> "
           << conn->LocalAddress().ToIpPort() << " is "
           << (conn->Connected() ? "UP" : "DOWN");
  if (conn->Connected()) {
    conn->SetTcpNoDelay(true);
    conn->Send(message_);
  }
}

void ChargenServer::OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
                              Timestamp time) {
  std::string msg(buf->RetrieveAllAsString());
  LOG_INFO << conn->name() << " discards " << msg.size()
           << " bytes received at " << time.ToString();
}

void ChargenServer::OnWriteComplete(const TcpConnectionPtr& conn) {
  transferred_ += message_.size();
  conn->Send(message_);
}

void ChargenServer::PrintThroughput() {
  Timestamp endTime = Timestamp::Now();
  double time = TimeDifference(endTime, start_time_);
  printf("%4.3f MiB/s\n",
         static_cast<double>(transferred_) / time / 1024 / 1024);
  transferred_ = 0;
  start_time_ = endTime;
}

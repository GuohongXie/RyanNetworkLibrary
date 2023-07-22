#include "event_loop.h"
#include "tcp_server.h"

void OnConnection(const TcpConnectionPtr& conn) {
  if (conn->Connected()) {
    conn->Shutdown();
  }
}

int main() {
  EventLoop loop;
  TcpServer server(&loop, InetAddress(1079), "Finger");
  server.SetConnectionCallback(OnConnection);
  server.Start();
  loop.Loop();
}

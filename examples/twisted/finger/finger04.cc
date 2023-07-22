#include "event_loop.h"
#include "tcp_server.h"


void OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
               Timestamp receiveTime) {
  if (buf->FindCRLF()) {
    conn->Shutdown();
  }
}

int main() {
  EventLoop loop;
  TcpServer server(&loop, InetAddress(1079), "Finger");
  server.SetMessageCallback(OnMessage);
  server.Start();
  loop.Loop();
}

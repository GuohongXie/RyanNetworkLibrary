#include "event_loop.h"
#include "tcp_server.h"


int main() {
  EventLoop loop;
  TcpServer server(&loop, InetAddress(1079), "Finger");
  server.Start();
  loop.Loop();
}

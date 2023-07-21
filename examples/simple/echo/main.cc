#include <unistd.h>

#include "echo.h"
#include "logging.h"
#include "event_loop.h"

int main() {
  LOG_INFO << "pid = " << ::getpid();
  EventLoop loop;
  InetAddress listen_addr(2007);
  EchoServer server(&loop, listen_addr);
  server.Start();
  loop.Loop();
}

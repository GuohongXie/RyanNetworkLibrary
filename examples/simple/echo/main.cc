#include <unistd.h>

#include "examples/simple/echo/echo.h"
#include "logger/logging.h"
#include "net/event_loop.h"

int main() {
  LOG_INFO << "pid = " << ::getpid();
  EventLoop loop;
  InetAddress listen_addr(2007);
  EchoServer server(&loop, listen_addr);
  server.Start();
  loop.Loop();
}

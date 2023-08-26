#include <unistd.h>

#include "examples/simple/chargen/chargen.h"

int main() {
  LOG_INFO << "pid = " << ::getpid();
  EventLoop loop;
  InetAddress listen_addr(2019);
  ChargenServer server(&loop, listen_addr, true);
  server.Start();
  loop.Loop();
}

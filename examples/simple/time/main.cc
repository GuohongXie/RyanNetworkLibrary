#include <unistd.h>

#include "time.h"
#include "logging.h"
#include "event_loop.h"


int main() {
  LOG_INFO << "pid = " << ::getpid();
  EventLoop loop;
  InetAddress listen_addr(2037);
  TimeServer server(&loop, listen_addr);
  server.Start();
  loop.Loop();
}

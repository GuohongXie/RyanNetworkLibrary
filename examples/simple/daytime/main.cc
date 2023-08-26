#include <unistd.h>

#include "examples/simple/daytime/daytime.h"
#include "logger/logging.h"
#include "net/event_loop.h"

int main() {
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress listenAddr(2013);
  DaytimeServer server(&loop, listenAddr);
  server.Start();
  loop.Loop();
}

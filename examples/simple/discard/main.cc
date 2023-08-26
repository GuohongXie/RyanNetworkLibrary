#include <unistd.h>

#include "examples/simple/discard/discard.h"
#include "logger/logging.h"
#include "net/event_loop.h"


int main() {
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress listenAddr(2009);
  DiscardServer server(&loop, listenAddr);
  server.Start();
  loop.Loop();
}

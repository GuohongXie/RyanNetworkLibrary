#include <cstdio>

#include "examples/idleconnection/echo.h"
#include "logger/logging.h"
#include "net/event_loop.h"


/*
void testHash()
{
  boost::hash<std::shared_ptr<int> > h;
  std::shared_ptr<int> x1(new int(10));
  std::shared_ptr<int> x2(new int(10));
  h(x1);
  assert(h(x1) != h(x2));
  x1 = x2;
  assert(h(x1) == h(x2));
  x1.reset();
  assert(h(x1) != h(x2));
  x2.reset();
  assert(h(x1) == h(x2));
}
*/

int main(int argc, char* argv[]) {
  // testHash();
  EventLoop loop;
  InetAddress listenAddr(2007);
  int idle_seconds = 10;
  if (argc > 1) {
    idle_seconds = atoi(argv[1]);
  }
  LOG_INFO << "pid = " << getpid() << ", idle seconds = " << idle_seconds;
  EchoServer server(&loop, listenAddr, idle_seconds);
  server.Start();
  loop.Loop();
}

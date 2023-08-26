#include <unistd.h>

#include "examples/simple/chargen/chargen.h"
#include "examples/simple/daytime/daytime.h"
#include "examples/simple/discard/discard.h"
#include "examples/simple/echo/echo.h"
#include "examples/simple/time/time.h"
#include "logger/logging.h"
#include "net/event_loop.h"


int main() {
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;  // one loop shared by multiple servers

  ChargenServer chargenServer(&loop, InetAddress(2019));
  chargenServer.Start();

  DaytimeServer daytimeServer(&loop, InetAddress(2013));
  daytimeServer.Start();

  DiscardServer discardServer(&loop, InetAddress(2009));
  discardServer.Start();

  EchoServer echoServer(&loop, InetAddress(2007));
  echoServer.Start();

  TimeServer timeServer(&loop, InetAddress(2037));
  timeServer.Start();

  loop.Loop();
}

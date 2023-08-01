#include "timer/timer_queue.h"

#include <cstdio>

#include <unistd.h>

#include "net/event_loop.h"
#include "net/event_loop_thread.h"
#include "base/thread.h"

int cnt = 0;
EventLoop* g_loop;

void PrintTid() {
  printf("pid = %d, tid = %d\n", getpid(), current_thread::Tid());
  printf("now %s\n", Timestamp::Now().ToString().c_str());
}

void Print(const char* msg) {
  printf("msg %s %s\n", Timestamp::Now().ToString().c_str(), msg);
  if (++cnt == 20) {
    g_loop->Quit();
  }
}

void Cancel(TimerId timer) {
  g_loop->Cancel(timer);
  printf("cancelled at %s\n", Timestamp::Now().ToString().c_str());
}

int main() {
  PrintTid();
  ::sleep(1);
  {
    EventLoop loop;
    g_loop = &loop;

    Print("main");
    loop.RunAfter(1, std::bind(Print, "once1"));
    loop.RunAfter(1.5, std::bind(Print, "once1.5"));
    loop.RunAfter(2.5, std::bind(Print, "once2.5"));
    loop.RunAfter(3.5, std::bind(Print, "once3.5"));
    TimerId t45 = loop.RunAfter(4.5, std::bind(Print, "once4.5"));
    loop.RunAfter(4.2, std::bind(Cancel, t45));
    loop.RunAfter(4.8, std::bind(Cancel, t45));
    loop.runEvery(2, std::bind(Print, "every2"));
    TimerId t3 = loop.runEvery(3, std::bind(Print, "every3"));
    loop.RunAfter(9.001, std::bind(Cancel, t3));

    loop.Loop();
    Print("main loop exits");
  }
  sleep(1);
  {
    EventLoopThread loop_thread;
    EventLoop* loop = loop_thread.StartLoop();
    loop->RunAfter(2, PrintTid);
    ::sleep(3);
    Print("thread loop exits");
  }
}

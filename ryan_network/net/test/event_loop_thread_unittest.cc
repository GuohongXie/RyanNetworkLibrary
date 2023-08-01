#include "net/event_loop_thread.h"

#include <cstdio>

#include <unistd.h>

#include "base/count_down_latch.h"
#include "net/event_loop.h"
#include "base/thread.h"

void Print(EventLoop* p = nullptr) {
  printf("Print: pid = %d, tid = %d, loop = %p\n", ::getpid(),
         current_thread::Tid(), p);
}

void Quit(EventLoop* p) {
  Print(p);
  p->Quit();
}

int main() {
  Print();

  {
    EventLoopThread thr1;  // never start
  }

  {
    // dtor calls quit()
    EventLoopThread thr2;
    EventLoop* loop = thr2.StartLoop();
    loop->RunInLoop(std::bind(Print, loop));
    current_thread::SleepUsec(500 * 1000);
  }

  {
    // quit() before dtor
    EventLoopThread thr3;
    EventLoop* loop = thr3.StartLoop();
    loop->RunInLoop(std::bind(Quit, loop));
    current_thread::SleepUsec(500 * 1000);
  }
}

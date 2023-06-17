#include "event_loop_thread.h"

#include <stdio.h>
#include <unistd.h>

#include "count_down_latch.h"
#include "event_loop.h"
#include "thread.h"

void Print(EventLoop* p = NULL) {
  printf("Print: pid = %d, tid = %d, loop = %p\n", ::getpid(),
         CurrentThread::Tid(), p);
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
    CurrentThread::SleepUsec(500 * 1000);
  }

  {
    // quit() before dtor
    EventLoopThread thr3;
    EventLoop* loop = thr3.StartLoop();
    loop->RunInLoop(std::bind(quit, loop));
    CurrentThread::SleepUsec(500 * 1000);
  }
}

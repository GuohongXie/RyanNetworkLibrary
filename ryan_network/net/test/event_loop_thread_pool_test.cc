#include "net/event_loop_thread_pool.h"

#include <cstdio>

#include <unistd.h>

#include "net/event_loop.h"
#include "base/thread.h"

void Print(EventLoop* p = nullptr) {
  printf("main(): pid = %d, tid = %d, loop = %p\n", ::getpid(),
         current_thread::Tid(), p);
}

void Init(EventLoop* p) {
  printf("Init(): pid = %d, tid = %d, loop = %p\n", ::getpid(),
         current_thread::Tid(), p);
}

int main() {
  Print();

  EventLoop loop;
  loop.RunAfter(11, std::bind(&EventLoop::Quit, &loop));

  {
    printf("Single thread %p:\n", &loop);
    EventLoopThreadPool model(&loop, "single");
    model.SetThreadNum(0);
    model.Start(Init);
    assert(model.GetNextLoop() == &loop);
    assert(model.GetNextLoop() == &loop);
    assert(model.GetNextLoop() == &loop);
  }

  {
    printf("Another thread:\n");
    EventLoopThreadPool model(&loop, "another");
    model.SetThreadNum(1);
    model.Start(Init);
    EventLoop* nextLoop = model.GetNextLoop();
    nextLoop->RunAfter(2, std::bind(Print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop == model.GetNextLoop());
    assert(nextLoop == model.GetNextLoop());
    ::sleep(3);
  }

  {
    printf("Three threads:\n");
    EventLoopThreadPool model(&loop, "three");
    model.SetThreadNum(3);
    model.Start(Init);
    EventLoop* nextLoop = model.GetNextLoop();
    nextLoop->RunInLoop(std::bind(Print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop != model.GetNextLoop());
    assert(nextLoop != model.GetNextLoop());
    assert(nextLoop == model.GetNextLoop());
  }

  loop.Loop();
}

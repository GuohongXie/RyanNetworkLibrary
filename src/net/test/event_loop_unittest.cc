#include "event_loop.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "thread.h"

EventLoop* g_loop;

void Callback() {
  printf("Callback(): pid = %d, tid = %d\n", ::getpid(), current_thread::Tid());
  EventLoop anotherLoop;
}

void ThreadFunc() {
  printf("ThreadFunc(): pid = %d, tid = %d\n", ::getpid(),
         current_thread::Tid());

  // assert(EventLoop::GetEventLoopOfCurrentThread() == nullptr);
  EventLoop loop;
  // assert(EventLoop::GetEventLoopOfCurrentThread() == &loop);
  loop.RunAfter(1.0, Callback);
  loop.Loop();
}

int main() {
  printf("main(): pid = %d, tid = %d\n", ::getpid(), current_thread::Tid());

  // assert(EventLoop::GetEventLoopOfCurrentThread() == nullptr);
  EventLoop loop;
  // assert(EventLoop::GetEventLoopOfCurrentThread() == &loop);

  Thread thread(ThreadFunc);
  thread.Start();

  loop.Loop();
}

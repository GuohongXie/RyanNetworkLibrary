#include "base/thread_pool.h"

#include <cstdio>
#include <functional>

#include <unistd.h>

#include "base/current_thread.h"
#include "logger/logging.h"

int count = 0;

void ShowInfo() { LOG_INFO << current_thread::Tid(); }

void test1() {
  ThreadPool pool;
  for (int i = 0; i < 5000; i++) {
    pool.AddTask(ShowInfo);
  }
  pool.AddTask([] { ::sleep(5); });
  pool.Start(4);
}

void InitFunc() { printf("create thread %d\n", ++count); }

int main() {
  test1();

  return 0;
}
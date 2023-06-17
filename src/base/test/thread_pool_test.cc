#include "thread_pool.h"

#include <unistd.h>

#include <cstdio>
#include <functional>

#include "current_thread.h"
#include "logging.h"

int count = 0;

void ShowInfo() { LOG_INFO << current_thread::Tid(); }

void test1() {
  ThreadPool pool;
  pool.SetThreadSize(4);
  for (int i = 0; i < 5000; i++) {
    pool.Add(ShowInfo);
  }
  pool.Add([] { ::sleep(5); });
  pool.Start();
}

void InitFunc() { printf("create thread %d\n", ++count); }

int main() {
  test1();

  return 0;
}
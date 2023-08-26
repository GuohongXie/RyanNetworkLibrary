#include <iostream>

#include "net/event_loop.h"

void Print(EventLoop* loop, int* count) {
  if (*count < 5) {
    std::cout << *count << "\n";
    ++(*count);

    loop->RunAfter(1, std::bind(Print, loop, count));
  } else {
    loop->Quit();
  }
}

int main() {
  EventLoop loop;
  int count = 0;
  // Note: loop.runEvery() is better for this use case.
  loop.RunAfter(1, std::bind(Print, &loop, &count));
  loop.Loop();
  std::cout << "Final count is " << count << "\n";
}

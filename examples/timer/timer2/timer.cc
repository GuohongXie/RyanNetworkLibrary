#include <iostream>

#include "net/event_loop.h"

void print() { std::cout << "Hello, world!\n"; }

int main() {
  EventLoop loop;
  loop.RunAfter(5, print);
  loop.Loop();
}

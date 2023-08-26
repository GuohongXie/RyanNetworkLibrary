#include <iostream>

#include "net/event_loop.h"

class Printer : Noncopyable {
 public:
  Printer(EventLoop* loop) : loop_(loop), count_(0) {
    // Note: loop.runEvery() is better for this use case.
    loop_->RunAfter(1, std::bind(&Printer::Print, this));
  }

  ~Printer() { std::cout << "Final count is " << count_ << "\n"; }

  void Print() {
    if (count_ < 5) {
      std::cout << count_ << "\n";
      ++count_;

      loop_->RunAfter(1, std::bind(&Printer::Print, this));
    } else {
      loop_->Quit();
    }
  }

 private:
  EventLoop* loop_;
  int count_;
};

int main() {
  EventLoop loop;
  Printer printer(&loop);
  loop.Loop();
}

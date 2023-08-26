#include <iostream>
#include <mutex>

#include "net/event_loop.h"
#include "net/event_loop_thread.h"

class Printer : Noncopyable {
 public:
  Printer(EventLoop* loop1, EventLoop* loop2)
      : loop1_(loop1), loop2_(loop2), count_(0) {
    loop1_->RunAfter(1, std::bind(&Printer::Print1, this));
    loop2_->RunAfter(1, std::bind(&Printer::Print2, this));
  }

  ~Printer() { std::cout << "Final count is " << count_ << "\n"; }

  void Print1() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (count_ < 10) {
      std::cout << "Timer 1: " << count_ << "\n";
      ++count_;

      loop1_->RunAfter(1, std::bind(&Printer::Print1, this));
    } else {
      loop1_->Quit();
    }
  }

  void Print2() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (count_ < 10) {
      std::cout << "Timer 2: " << count_ << "\n";
      ++count_;

      loop2_->RunAfter(1, std::bind(&Printer::Print2, this));
    } else {
      loop2_->Quit();
    }
  }

 private:
  std::mutex mutex_;
  EventLoop* loop1_;
  EventLoop* loop2_;
  int count_;
};

int main() {
  std::unique_ptr<Printer>
      printer;  // make sure printer lives longer than loops, to avoid
                // race condition of calling Print2() on destructed object.
  EventLoop loop;
  EventLoopThread loopThread;
  EventLoop* loopInAnotherThread = loopThread.StartLoop();
  printer.reset(new Printer(&loop, loopInAnotherThread));
  loop.Loop();
}

#include <stdio.h>
#include <mutex>

#include "net/event_loop.h"
#include "net/event_loop_thread.h"


//
// Minimize locking
//

class Printer : Noncopyable {
 public:
  Printer(EventLoop* loop1, EventLoop* loop2)
      : loop1_(loop1), loop2_(loop2), count_(0) {
    loop1_->RunAfter(1, std::bind(&Printer::Print1, this));
    loop2_->RunAfter(1, std::bind(&Printer::Print2, this));
  }

  ~Printer() {
    // cout is not thread safe
    // std::cout << "Final count is " << count_ << "\n";
    printf("Final count is %d\n", count_);
  }

  void Print1() {
    bool should_quit = false;
    int count = 0;

    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (count_ < 10) {
        count = count_;
        ++count_;
      } else {
        should_quit = true;
      }
    }

    // out of lock
    if (should_quit) {
      // printf("loop1_->quit()\n");
      loop1_->Quit();
    } else {
      // cout is not thread safe
      // std::cout << "Timer 1: " << count << "\n";
      printf("Timer 1: %d\n", count);
      loop1_->RunAfter(1, std::bind(&Printer::Print1, this));
    }
  }

  void Print2() {
    bool should_quit = false;
    int count = 0;

    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (count_ < 10) {
        count = count_;
        ++count_;
      } else {
        should_quit = true;
      }
    }

    // out of lock
    if (should_quit) {
      // printf("loop2_->quit()\n");
      loop2_->Quit();
    } else {
      // cout is not thread safe
      // std::cout << "Timer 2: " << count << "\n";
      printf("Timer 2: %d\n", count);
      loop2_->RunAfter(1, std::bind(&Printer::Print2, this));
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
  EventLoopThread loop_thread;
  EventLoop* loop_in_another_thread = loop_thread.StartLoop();
  printer.reset(new Printer(&loop, loop_in_another_thread));
  loop.Loop();
}

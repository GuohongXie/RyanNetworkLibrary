#include "base/thread.h"

#include <cstdio>
#include <string>

#include <unistd.h>

#include "base/current_thread.h"

void Mysleep(int seconds) {
  timespec t = {seconds, 0};
  nanosleep(&t, NULL);
}

void ThreadFunc() { printf("Tid=%d\n", current_thread::Tid()); }

void ThreadFunc2(int x) { printf("Tid=%d, x=%d\n", current_thread::Tid(), x); }

void ThreadFunc3() {
  printf("Tid=%d\n", current_thread::Tid());
  Mysleep(1);
}

class Foo {
 public:
  explicit Foo(double x) : x_(x) {}

  void MemberFunc() {
    printf("Tid=%d, Foo::x_=%f\n", current_thread::Tid(), x_);
  }

  void MemberFunc2(const std::string& text) {
    printf("Tid=%d, Foo::x_=%f, text=%s\n", current_thread::Tid(), x_,
           text.c_str());
  }

 private:
  double x_;
};

int main() {
  printf("pid=%d, Tid=%d\n", ::getpid(), current_thread::Tid());

  Thread t1(ThreadFunc);
  t1.Start();
  printf("t1.Tid=%d\n", t1.tid());
  t1.Join();

  Thread t2(std::bind(ThreadFunc2, 42),
            "thread for free function with argument");
  t2.Start();
  printf("t2.Tid=%d\n", t2.tid());
  t2.Join();

  Foo foo(87.53);
  Thread t3(std::bind(&Foo::MemberFunc, &foo),
            "thread for member function without argument");
  t3.Start();
  t3.Join();

  Thread t4(
      std::bind(&Foo::MemberFunc2, std::ref(foo), std::string("ryan")));
  t4.Start();
  t4.Join();

  {
    Thread t5(ThreadFunc3);
    t5.Start();
    // t5 may destruct eariler than thread creation.
  }
  Mysleep(2);
  {
    Thread t6(ThreadFunc3);
    t6.Start();
    Mysleep(2);
    // t6 destruct later than thread creation.
  }
  sleep(2);
  printf("number of created threads %d\n", Thread::num_created());
}

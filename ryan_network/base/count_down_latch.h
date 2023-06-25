#ifndef RYANLIB_BASE_COUNT_DOWN_LATCH_H_
#define RYANLIB_BASE_COUNT_DOWN_LATCH_H_

#include <condition_variable>
#include <mutex>

#include "noncopyable.h"

class CountDownLatch : public Noncopyable {
 public:
  explicit CountDownLatch(int count);

  void Wait();

  void CountDown();

  int GetCount() const;

 private:
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  int count_;
};

#endif  // RYANLIB_BASE_COUNT_DOWN_LATCH_H_

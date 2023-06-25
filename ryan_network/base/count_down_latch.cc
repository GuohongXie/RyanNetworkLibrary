#include "count_down_latch.h"

CountDownLatch::CountDownLatch(int count) : count_(count) {}

void CountDownLatch::Wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (count_ > 0) {
    cv_.wait(lock);
  }
}

void CountDownLatch::CountDown() {
  std::unique_lock<std::mutex> lock(mutex_);
  --count_;
  if (count_ == 0) {
    cv_.notify_all();
  }
}

int CountDownLatch::GetCount() const {
  std::unique_lock<std::mutex> lock(mutex_);
  return count_;
}

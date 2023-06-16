#ifndef RYANLIB_BASE_THREAD_POOL_H_
#define RYANLIB_BASE_THREAD_POOL_H_

#include <condition_variable>
#include <deque>
#include <mutex>
#include <vector>

#include "logging.h"
#include "thread.h"
#include "noncopyable.h"

class ThreadPool : Noncopyable {
 public:
  using ThreadFunction = std::function<void()>;

  explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
  ~ThreadPool();

  void SetThreadInitCallback(const ThreadFunction& cb) {
    thread_init_callback_ = cb;
  }
  void SetThreadSize(const int& num) { thread_size_ = num; }
  void Start();
  void Stop();

  const std::string& name() const { return name_; }
  size_t QueueSize() const;

  void Add(ThreadFunction ThreadFunction);

 private:
  bool IsFull() const;
  void RunInThread();

  mutable std::mutex mutex_;
  std::condition_variable cond_;
  std::string name_;
  ThreadFunction thread_init_callback_;
  std::vector<std::unique_ptr<Thread>> threads_;
  std::deque<ThreadFunction> queue_;
  bool running_;
  size_t thread_size_;
};

#endif  // RYANLIB_BASE_THREAD_POOL_H_

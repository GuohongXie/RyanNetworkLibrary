#ifndef RYANLIB_BASE_THREAD_POOL_H_
#define RYANLIB_BASE_THREAD_POOL_H_

#include <vector>
#include <deque>
#include <mutex>
#include <string>
#include <condition_variable>

#include "logger/logging.h"
#include "base/thread.h"


class ThreadPool : Noncopyable {
 public:
  typedef std::function<void()> Task;

  explicit ThreadPool(const std::string& name_arg = std::string("ThreadPool"));
  ~ThreadPool();

  // Must be called before start().
  void SetMaxQueueSize(int max_size) { max_queue_size_ = max_size; }
  void SetThreadInitCallback(const Task& cb) { thread_init_callback_ = cb; }

  void Start(int num_threads);
  void Stop();

  const std::string& name() const { return name_; }

  size_t QueueSize() const;

  void RunTask(Task task);

 private:
  bool IsFull() const;
  void RunInThread();
  Task Take();

  mutable std::mutex mutex_;
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
  std::string name_;
  Task thread_init_callback_;
  std::vector<std::unique_ptr<Thread>> threads_;
  std::deque<Task> queue_;
  size_t max_queue_size_;
  bool running_;
};

#endif  // RYANLIB_BASE_THREAD_POOL_H_
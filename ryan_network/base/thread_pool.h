#ifndef RYANLIB_BASE_THREAD_POOL_H_
#define RYANLIB_BASE_THREAD_POOL_H_

#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>

#include "logger/logging.h"
#include "base/noncopyable.h"
#include "base/thread.h"

class ThreadPool : Noncopyable {
 public:
  using ThreadFunction = std::function<void()>;

  explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
  ~ThreadPool();

  void SetThreadInitCallback(const ThreadFunction& cb) {
    thread_init_callback_ = cb;
  }
  void Start(int num_threads);
  void Stop();

  const std::string& name() const { return name_; }
  size_t QueueSize() const;

  void AddTask(ThreadFunction ThreadFunction);

 private:
  //暂时只提供在Start中设置num_threads的版本，故这两个函数暂时先不暴露接口
  //隐藏起来
  void Start();
  void set_num_threads(int num) { num_threads_ = num; }

  bool IsFull() const;
  void RunInThread();

  mutable std::mutex mutex_;
  std::condition_variable cond_;
  std::string name_;
  ThreadFunction thread_init_callback_;
  std::vector<std::unique_ptr<Thread>> threads_;
  std::deque<ThreadFunction> queue_;
  bool running_;
  size_t num_threads_;
};

#endif  // RYANLIB_BASE_THREAD_POOL_H_

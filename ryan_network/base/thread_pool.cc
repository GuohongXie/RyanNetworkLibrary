#include "thread_pool.h"

ThreadPool::ThreadPool(const std::string& name)
    : mutex_(), cond_(), name_(name), running_(false) {}

ThreadPool::~ThreadPool() {
  Stop();
  for (const auto& t : threads_) {
    // 等待直到线程结束
    t->Join();
  }
}

void ThreadPool::Start() {
  running_ = true;
  threads_.reserve(thread_size_);
  for (int i = 0; i < thread_size_; ++i) {
    char id[32];
    snprintf(id, sizeof(id), "%d", i + 1);
    threads_.emplace_back(
        new Thread(std::bind(&ThreadPool::RunInThread, this), name_ + id));
    threads_[i]->Start();
  }
  // 不创建新线程
  if (thread_size_ == 0 && thread_init_callback_) {
    thread_init_callback_();
  }
}

void ThreadPool::Stop() {
  std::lock_guard<std::mutex> lock(mutex_);
  running_ = false;
  cond_.notify_all();  // 唤醒所有线程
}

size_t ThreadPool::QueueSize() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.size();
}

void ThreadPool::Add(ThreadFunction ThreadFunction) {
  std::unique_lock<std::mutex> lock(mutex_);
  queue_.push_back(ThreadFunction);
  cond_.notify_one();
}

void ThreadPool::RunInThread() {
  try {
    if (thread_init_callback_) {
      thread_init_callback_();
    }
    ThreadFunction task;
    // 之前写成了 while (true)，这会导致出不去循环
    while (true) {
      {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty()) {
          if (!running_) {
            return;
          }
          cond_.wait(lock);
        }
        task = queue_.front();
        queue_.pop_front();
      }
      if (task != nullptr) {
        task();
      }
    }
  } catch (...) {
    LOG_WARN << "RunInThread throw exception";
  }
}
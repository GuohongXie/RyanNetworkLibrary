#include "base/thread_pool.h"

#include <cassert>
#include <cstdio>

ThreadPool::ThreadPool(const std::string& nameArg)
    : mutex_(),
      not_empty_(),
      not_full_(),
      name_(nameArg),
      max_queue_size_(0),
      running_(false) {}

ThreadPool::~ThreadPool() {
  if (running_) {
    Stop();
  }
}

void ThreadPool::Start(int numThreads) {
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(numThreads);
  for (int i = 0; i < numThreads; ++i) {
    char id[32];
    snprintf(id, sizeof(id), "%d", i + 1);
    threads_.emplace_back(new Thread(
        std::bind(&ThreadPool::RunInThread, this), name_ + id));
    threads_[i]->Start();
  }
  if (numThreads == 0 && thread_init_callback_) {
    thread_init_callback_();
  }
}

void ThreadPool::Stop() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
    not_empty_.notify_all();
    not_full_.notify_all();
  }
  for (auto& thr : threads_) {
    thr->Join();
  }
}

size_t ThreadPool::QueueSize() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.size();
}

void ThreadPool::RunTask(Task task) {
  if (threads_.empty()) {
    task();
  } else {
    std::unique_lock<std::mutex> lock(mutex_);
    while (IsFull() && running_) {
      not_full_.wait(lock);
    }
    if (!running_) return;
    assert(!IsFull());

    queue_.push_back(std::move(task));
    not_empty_.notify_one();
  }
}

ThreadPool::Task ThreadPool::Take() {
  std::unique_lock<std::mutex> lock(mutex_);
  // always use a while-loop, due to spurious wakeup
  while (queue_.empty() && running_) {
    not_empty_.wait(lock);
  }
  Task task;
  if (!queue_.empty()) {
    task = queue_.front();
    queue_.pop_front();
    if (max_queue_size_ > 0) {
      not_full_.notify_one();
    }
  }
  return task;
}

bool ThreadPool::IsFull() const {
  return max_queue_size_ > 0 && queue_.size() >= max_queue_size_;
}

void ThreadPool::RunInThread() {
  try {
    if (thread_init_callback_) {
      thread_init_callback_();
    }
    while (running_) {
      Task task(Take());
      if (task) {
        task();
      }
    }
  } catch (const std::exception& ex) {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  } catch (...) {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n",
            name_.c_str());
    throw;  // rethrow
  }
}
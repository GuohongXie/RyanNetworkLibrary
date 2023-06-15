#ifndef RYANLIB_BASE_THREAD_H_
#define RYANLIB_BASE_THREAD_H_

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "noncopyable.h"

class Thread : public Noncopyable {
 public:
  using ThreadFunc = std::function<void()>;
  explicit Thread(ThreadFunc, const std::string& name = std::string());
  ~Thread();

  void Start();
  void Join();

  bool started() const { return started_; }
  pid_t tid() const { return tid_; }
  const std::string& name() const {
    return name_;
  }  //此处为什么用const修饰返回值

  static int num_created() { return num_created_; }

 private:
  void SetDefaultName();

  bool started_;  //线程是否启动
  bool joined_;
  std::shared_ptr<std::thread> thread_;  //用智能指针托管线程资源
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  static std::atomic_int32_t num_created_;
};

#endif  // RYANLIB_BASE_THREAD_H_
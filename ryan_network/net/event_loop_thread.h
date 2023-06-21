#ifndef RYANLIB_NET_EVENT_LOOP_THREAD_H_
#define RYANLIB_NET_EVENT_LOOP_THREAD_H_

#include <condition_variable>
#include <functional>
#include <mutex>

#include "thread.h"
#include "noncopyable.h"

// one loop per thread
class EventLoop;
class EventLoopThread : Noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const std::string& name = std::string());
  ~EventLoopThread();

  EventLoop* StartLoop();  // 开启线程池

 private:
  void ThreadFunc();

  // EventLoopThread::threadFunc 会创建 EventLoop
  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
  ThreadInitCallback callback_;
};

#endif  // RYANLIB_NET_EVENT_LOOP_THREAD_H_
#ifndef RYANLIB_NET_EVENT_LOOP_H_
#define RYANLIB_NET_EVENT_LOOP_H_

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "current_thread.h"
#include "noncopyable.h"
#include "timer_queue.h"
#include "timestamp.h"

class Channel;
class Poller;
// 事件循环类 主要包含了两大模块，channel poller
class EventLoop : Noncopyable {
 public:
  using Functor = std::function<void()>;

  EventLoop();
  ~EventLoop();

  void Loop();
  void Quit();

  Timestamp poll_return_time() const { return poll_return_time_; }

  // 在当前线程同步调用函数
  void RunInLoop(Functor cb);
  /**
   * 把cb放入队列，唤醒loop所在的线程执行cb
   *
   * 实例情况：
   * 在mainLoop中获取subLoop指针，然后调用相应函数
   * 在queueLoop中发现当前的线程不是创建这个subLoop的线程，将此函数装入subLoop的pendingFunctors容器中
   * 之后mainLoop线程会调用subLoop::wakeup向subLoop的eventFd写数据，以此唤醒subLoop来执行pengdingFunctors
   */
  void QueueInLoop(Functor cb);

  // 用来唤醒loop所在的线程
  void Wakeup();

  // EventLoop的方法 => Poller
  void UpdateChannel(Channel* channel);
  void RemoveChannel(Channel* channel);
  bool HasChannel(Channel* channel);

  // 判断EventLoop是否在自己的线程
  bool IsInLoopThread() const { return thread_id_ == current_thread::Tid(); }
  // void AssertInLoopThread() {
  //  if (!IsInLoopThread()) {
  //    AbortNotInLoopThread();
  //  }
  //}

  /**
   * 定时任务相关函数
   */
  void RunAt(Timestamp timestamp, Functor&& cb) {
    timer_queue_->AddTimer(std::move(cb), timestamp, 0.0);
  }

  void RunAfter(double waitTime, Functor&& cb) {
    Timestamp time(AddTime(Timestamp::Now(), waitTime));
    RunAt(time, std::move(cb));
  }

  void RunEvery(double interval, Functor&& cb) {
    Timestamp timestamp(AddTime(Timestamp::Now(), interval));
    timer_queue_->AddTimer(std::move(cb), timestamp, interval);
  }

  //  static EventLoop* GetEventLoopOfCurrentThread();

 private:
  // void AbortNotInLoopThread();
  void HandleRead();
  void DoPendingFunctors();

  using ChannelList = std::vector<Channel*>;
  std::atomic_bool looping_;  // 原子操作，通过CAS实现
  std::atomic_bool quit_;     // 标志退出事件循环
  std::atomic_bool
      calling_pending_functors_;  // 标志当前loop是否有需要执行的回调操作
  const pid_t thread_id_;       // 记录当前loop所在线程的id
  Timestamp poll_return_time_;  // poller返回发生事件的channels的返回时间
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timer_queue_;

  /**
   * TODO:eventfd用于线程通知机制，libevent和我的webserver是使用sockepair
   * 作用：当mainLoop获取一个新用户的Channel 需通过轮询算法选择一个subLoop
   * 通过该成员唤醒subLoop处理Channel
   */
  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;

  ChannelList active_channels_;      // 活跃的Channel
  Channel* current_active_channel_;  // 当前处理的活跃channel
  std::mutex mutex_;  // 用于保护pendingFunctors_线程安全操作
  std::vector<Functor>
      pending_functors_;  // 存储loop跨线程需要执行的所有回调操作
};

#endif  // RYANLIB_NET_EVENT_LOOP_H_
#include "event_loop.h"

#include <fcntl.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "logging.h"
#include "poller.h"

// 防止一个线程创建多个EventLoop (thread_local)
__thread EventLoop* t_loop_in_this_thread = nullptr;

// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

// TODO:eventfd使用
int CreateEventfd() {
  int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evfd < 0) {
    LOG_FATAL << "eventfd error: " << errno;
  }
  return evfd;
}

// EventLoop* EventLoop::GetEventLoopOfCurrentThread() {
//  return t_loop_in_this_thread;
//}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      calling_pending_functors_(false),
      thread_id_(current_thread::Tid()),
      poller_(Poller::NewDefaultPoller(this)),
      timer_queue_(new TimerQueue(this)),
      wakeup_fd_(CreateEventfd()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      current_active_channel_(nullptr) {
  LOG_DEBUG << "EventLoop created " << this << " the index is " << thread_id_;
  LOG_DEBUG << "EventLoop created wakeupFd " << wakeup_channel_->fd();
  if (t_loop_in_this_thread) {
    LOG_FATAL << "Another EventLoop" << t_loop_in_this_thread
              << " exists in this thread " << thread_id_;
  } else {
    t_loop_in_this_thread = this;
  }

  // 设置wakeupfd的事件类型以及发生事件的回调函数
  wakeup_channel_->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
  // 每一个EventLoop都将监听wakeupChannel的EPOLLIN事件
  wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {
  // channel移除所有感兴趣事件
  wakeup_channel_->DisableAll();
  // 将channel从EventLoop中删除
  wakeup_channel_->Remove();
  // 关闭 wakeup_fd_
  ::close(wakeup_fd_);
  // 指向EventLoop指针为空
  t_loop_in_this_thread = nullptr;
}

void EventLoop::Loop() {
  looping_ = true;
  quit_ = false;

  LOG_INFO << "EventLoop " << this << " start looping";

  while (!quit_) {
    // 清空activeChannels_
    active_channels_.clear();
    // 获取
    poll_return_time_ = poller_->Poll(kPollTimeMs, &active_channels_);
    for (Channel* channel : active_channels_) {
      channel->HandleEvent(poll_return_time_);
    }
    // 执行当前EventLoop事件循环需要处理的回调操作
    /**
     * IO thread：mainLoop accept fd 打包成 chennel 分发给 subLoop
     * mainLoop实现注册一个回调，交给subLoop来执行，wakeup subLoop
     * 之后，让其执行注册的回调操作 这些回调函数在 std::vector<Functor>
     * pending_functors_; 之中
     */
    DoPendingFunctors();
  }
  looping_ = false;
}

void EventLoop::Quit() {
  quit_ = true;

  /**
   * TODO:生产者消费者队列派发方式和muduo的派发方式
   * 有可能是别的线程调用quit(调用线程不是生成EventLoop对象的那个线程)
   * 比如在工作线程(subLoop)中调用了IO线程(mainLoop)
   * 这种情况会唤醒主线程
   */
  if (IsInLoopThread()) {
    Wakeup();
  }
}

// 在当前eventLoop中执行回调函数
void EventLoop::RunInLoop(Functor cb) {
  // 每个EventLoop都保存创建自己的线程tid
  // 我们可以通过CurrentThread::tid()获取当前执行线程的tid然后和EventLoop保存的进行比较
  if (IsInLoopThread()) {
    cb();
  }
  // 在非当前eventLoop线程中执行回调函数，需要唤醒evevntLoop所在线程
  else {
    QueueInLoop(cb);
  }
}

void EventLoop::QueueInLoop(Functor cb) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    pending_functors_.emplace_back(cb);  // 使用了std::move
  }

  // 唤醒相应的，需要执行上面回调操作的loop线程
  /**
   * TODO:
   * std::atomic_bool callingPendingFunctors_;
   * 标志当前loop是否有需要执行的回调操作 这个 || callingPendingFunctors_
   * 比较有必要，因为在执行回调的过程可能会加入新的回调
   * 则这个时候也需要唤醒，否则就会发生有事件到来但是仍被阻塞住的情况
   */
  if (!IsInLoopThread() || calling_pending_functors_) {
    // 唤醒loop所在的线程
    Wakeup();
  }
}

// void EventLoop::AbortNotInLoopThread() {
//  LOG_FATAL << "EventLoop::AbortNotInLoopThread - EventLoop " << this
//            << " was created in thread_id_ = " << thread_id_
//            << ", current thread id = " << current_thread::Tid();
//}

void EventLoop::Wakeup() {
  uint64_t one = 1;
  ssize_t n = ::write(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG_ERROR << "EventLoop::wakeup writes " << n << " bytes instead of 8";
  }
}

void EventLoop::HandleRead() {
  uint64_t one = 1;
  ssize_t n = ::read(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::UpdateChannel(Channel* channel) {
  poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
  poller_->RemoveChannel(channel);
}

bool EventLoop::HasChannel(Channel* channel) {
  return poller_->HasChannel(channel);
}

void EventLoop::DoPendingFunctors() {
  std::vector<Functor> functors;
  calling_pending_functors_ = true;

  /**
   * TODO:
   * 如果没有生成这个局部的 functors
   * 则在互斥锁加持下，我们直接遍历pendingFunctors
   * 其他线程这个时候无法访问，无法向里面注册回调函数，增加服务器时延
   */
  {
    std::unique_lock<std::mutex> lock(mutex_);
    functors.swap(pending_functors_);
  }

  for (const Functor& functor : functors) {
    functor();
  }

  calling_pending_functors_ = false;
}

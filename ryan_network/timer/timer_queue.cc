#include "timer/timer_queue.h"

#include <cstring>

#include <sys/timerfd.h>
#include <unistd.h>

#include "net/channel.h"
#include "net/event_loop.h"
#include "logger/logging.h"
#include "timer/timer.h"

int CreateTimerfd() {
  /**
   * CLOCK_MONOTONIC：绝对时间
   * TFD_NONBLOCK：非阻塞
   */
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0) {
    LOG_ERROR << "Failed in timerfd_create";
  }
  return timerfd;
}

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(CreateTimerfd()),
      timerfd_channel_(loop_, timerfd_),
      timers_() {
  timerfd_channel_.SetReadCallback(std::bind(&TimerQueue::HandleRead, this));
  timerfd_channel_.EnableReading();
}

TimerQueue::~TimerQueue() {
  timerfd_channel_.DisableAll();
  timerfd_channel_.Remove();
  ::close(timerfd_);
  // 删除所有定时器
  for (const Entry& timer : timers_) {
    delete timer.second;
  }
}

void TimerQueue::AddTimer(TimerCallback cb, Timestamp when, double interval) {
  Timer* timer = new Timer(std::move(cb), when, interval);
  loop_->RunInLoop(std::bind(&TimerQueue::AddTimerInLoop, this, timer));
}

void TimerQueue::AddTimerInLoop(Timer* timer) {
  // 是否取代了最早的定时触发时间
  bool eraliestChanged = Insert(timer);

  // 我们需要重新设置timerfd_触发时间
  if (eraliestChanged) {
    ResetTimerfd(timerfd_, timer->expiration());
  }
}

// 重置timerfd
void TimerQueue::ResetTimerfd(int timerfd_, Timestamp expiration) {
  struct itimerspec newValue;
  struct itimerspec oldValue;
  memset(&newValue, '\0', sizeof(newValue));
  memset(&oldValue, '\0', sizeof(oldValue));

  // 超时时间 - 现在时间
  int64_t microSecondDif = expiration.micro_seconds_since_epoch() -
                           Timestamp::Now().micro_seconds_since_epoch();
  if (microSecondDif < 100) {
    microSecondDif = 100;
  }

  struct timespec ts;
  ts.tv_sec =
      static_cast<time_t>(microSecondDif / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microSecondDif % Timestamp::kMicroSecondsPerSecond) * 1000);
  newValue.it_value = ts;
  // 此函数会唤醒事件循环
  if (::timerfd_settime(timerfd_, 0, &newValue, &oldValue)) {
    LOG_ERROR << "timerfd_settime faield()";
  }
}

void ReadTimerFd(int timerfd) {
  uint64_t read_byte;
  ssize_t readn = ::read(timerfd, &read_byte, sizeof(read_byte));

  if (readn != sizeof(read_byte)) {
    LOG_ERROR << "TimerQueue::ReadTimerFd read_size < 0";
  }
}

// 返回删除的定时器节点 （std::vector<Entry> expired）
std::vector<TimerQueue::Entry> TimerQueue::GetExpired(Timestamp now) {
  std::vector<Entry> expired;
  // TODO:???
  Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  TimerList::iterator end = timers_.lower_bound(sentry);
  std::copy(timers_.begin(), end, back_inserter(expired));
  timers_.erase(timers_.begin(), end);

  return expired;
}

void TimerQueue::HandleRead() {
  Timestamp now = Timestamp::Now();
  ReadTimerFd(timerfd_);

  std::vector<Entry> expired = GetExpired(now);

  // 遍历到期的定时器，调用回调函数
  calling_expired_timers_ = true;
  for (const Entry& it : expired) {
    it.second->Run();
  }
  calling_expired_timers_ = false;

  // 重新设置这些定时器
  Reset(expired, now);
}

void TimerQueue::Reset(const std::vector<Entry>& expired, Timestamp now) {
  Timestamp nextExpire;
  for (const Entry& it : expired) {
    // 重复任务则继续执行
    if (it.second->repeat()) {
      auto timer = it.second;
      timer->Restart(Timestamp::Now());
      Insert(timer);
    } else {
      delete it.second;
    }

    // 如果重新插入了定时器，需要继续重置timerfd
    if (!timers_.empty()) {
      ResetTimerfd(timerfd_, (timers_.begin()->second)->expiration());
    }
  }
}

bool TimerQueue::Insert(Timer* timer) {
  bool earliestChanged = false;
  Timestamp when = timer->expiration();
  TimerList::iterator it = timers_.begin();
  if (it == timers_.end() || when < it->first) {
    // 说明最早的定时器已经被替换了
    earliestChanged = true;
  }

  // 定时器管理红黑树插入此新定时器
  timers_.insert(Entry(when, timer));

  return earliestChanged;
}

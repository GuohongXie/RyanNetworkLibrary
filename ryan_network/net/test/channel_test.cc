#include "net/channel.h"

#include <cstdio>
#include <functional>
#include <map>

#include <sys/timerfd.h>
#include <unistd.h>

#include "net/event_loop.h"
#include "net/callback.h"
#include "logger/logging.h"

void Print(const char* msg) {
  static std::map<const char*, Timestamp> lasts;
  Timestamp& last = lasts[msg];
  Timestamp now = Timestamp::Now();
  printf("%s tid %d %s delay %f\n", now.ToString().c_str(),
         current_thread::Tid(), msg, TimeDifference(now, last));
  last = now;
}

int CreateTimerfd();
void ReadTimerfd(int timerfd, Timestamp now);

// Use relative time, immunized to wall clock changes.
class PeriodicTimer {
 public:
  PeriodicTimer(EventLoop* loop, double interval, const TimerCallback& cb)
      : loop_(loop),
        timerfd_(CreateTimerfd()),
        timerfd_channel_(loop, timerfd_),
        interval_(interval),
        cb_(cb) {
    timerfd_channel_.SetReadCallback(
        std::bind(&PeriodicTimer::HandleRead, this));
    timerfd_channel_.EnableReading();
  }

  void Start() {
    struct itimerspec spec;
    ::memset(&spec, 0, sizeof(spec));
    spec.it_interval = ToTimeSpec(interval_);
    spec.it_value = spec.it_interval;
    int ret = ::timerfd_settime(timerfd_, 0 /* relative timer */, &spec, NULL);
    if (ret) {
      LOG_ERROR << "timerfd_settime()";
    }
  }

  ~PeriodicTimer() {
    timerfd_channel_.DisableAll();
    timerfd_channel_.Remove();
    ::close(timerfd_);
  }

 private:
  typedef std::function<void()> TimerCallback;
  void HandleRead() {
    loop_->AssertInLoopThread();
    ReadTimerfd(timerfd_, Timestamp::Now());
    if (cb_) cb_();
  }

  static struct timespec ToTimeSpec(double seconds) {
    struct timespec ts;
    ::memset(&ts, 0, sizeof(ts));
    const int64_t kNanoSecondsPerSecond = 1000000000;
    const int kMinInterval = 100000;
    int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
    if (nanoseconds < kMinInterval) nanoseconds = kMinInterval;
    ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(nanoseconds % kNanoSecondsPerSecond);
    return ts;
  }

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfd_channel_;
  const double interval_;  // in seconds
  TimerCallback cb_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid() << ", tid = " << current_thread::Tid()
           << " Try adjusting the wall clock, see what happens.";
  EventLoop loop;
  PeriodicTimer timer(&loop, 1, std::bind(Print, "PeriodicTimer"));
  timer.Start();
  loop.RunEvery(1, std::bind(Print, "EventLoop::runEvery"));
  loop.Loop();
}

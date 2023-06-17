#ifndef RYANLIB_BASE_TIMESTAMP_H_
#define RYANLIB_BASE_TIMESTAMP_H_

#include <sys/time.h>

#include <iostream>
#include <string>
class Timestamp {
 public:
  Timestamp() : micro_seconds_since_epoch_(0) {}
  explicit Timestamp(int64_t micro_seconds_since_epoch)
      : micro_seconds_since_epoch_(micro_seconds_since_epoch) {}

  // 获取当前时间戳
  static Timestamp Now();

  //用std::string形式返回，格式[millisec].[microsec]
  std::string ToString() const;
  //格式, "%4d年%02d月%2d日 星期%d %02d:%02d:%02d.%06d", 时分秒.微秒
  std::string ToFormattedString(bool show_micro_seconds = false) const;

  //返回当前时间戳的微秒
  int64_t micro_seconds_since_epoch() const {
    return micro_seconds_since_epoch_;
  }

  //返回当前时间的秒数
  time_t seconds_since_epoch() const {
    return static_cast<time_t>(micro_seconds_since_epoch_ /
                               kMicroSecondsPerSecond);
  }

  //失效的时间戳，返回一个值为0的Timestamp
  static Timestamp Invalid() { return Timestamp(); }

  static const int kMicroSecondsPerSecond = 1000 * 1000;

 private:
  //自epoch开始的微妙数
  // UNIX时间戳：在Unix和类Unix系统中，
  // epoch通常指的是从协调世界时（Coordinated Universal Time，UTC）
  //的 1970 年 1 月 1 日 00:00:00 开始经过的秒数
  int64_t micro_seconds_since_epoch_;
};

//这里传值或者传引用都可以，因为Timestamp类只有一个non-static成员变量
inline bool operator<(Timestamp& lhs, Timestamp& rhs) {
  return lhs.micro_seconds_since_epoch() < rhs.micro_seconds_since_epoch();
}

inline bool operator==(Timestamp& lhs, Timestamp& rhs) {
  return lhs.micro_seconds_since_epoch() == rhs.micro_seconds_since_epoch();
}

//如果是重复定时任务就会对此时间戳进行增加
inline Timestamp AddTime(Timestamp timestamp, double seconds) {
  //将延时的秒数设置为微秒
  int64_t delta =
      static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  //返回新增时后的时间戳
  return Timestamp(timestamp.micro_seconds_since_epoch() + delta);
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microsecond
/// resolution for next 100 years.
inline double TimeDifference(Timestamp high, Timestamp low)
{
  int64_t diff = high.micro_seconds_since_epoch() - low.micro_seconds_since_epoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}
#endif  // RYANLIB_BASE_TIMESTAMP_H_

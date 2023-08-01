#include "base/timestamp.h"

//获取当前时间戳
Timestamp Timestamp::Now() {
  struct timeval tv;  // 此struct在sys/time.h中定义
  // 获取微秒和秒
  // 在x86-64平台上gettimeofday()已经不是系统调用，不会陷入内核，多次调用不会有性能损失
  gettimeofday(&tv, nullptr);
  int64_t seconds = tv.tv_sec;
  //转换为微秒
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

//std::string Timestamp::ToString() const {
//  char buf[32] = {0};
//  int64_t seconds = micro_seconds_since_epoch_ / kMicroSecondsPerSecond;
//  int64_t microseconds = micro_seconds_since_epoch_ % kMicroSecondsPerSecond;
//  snprintf(buf, sizeof(buf), "%ld.%06ld", seconds, microseconds);
//  return buf;
//}

// 2023/06/12 19:44:28
// 2023/06/12 19:44:28.952562

std::string Timestamp::ToFormattedString(bool show_micro_seconds) const {
  char buf[64] = {0};
  time_t seconds =
      static_cast<time_t>(micro_seconds_since_epoch_ / kMicroSecondsPerSecond);
  //使用localtime函数将秒数格式化成日历时间
  tm* tm_time = localtime(&seconds);
  if (show_micro_seconds) {
    int micro_seconds =
        static_cast<int>(micro_seconds_since_epoch_ % kMicroSecondsPerSecond);
    snprintf(buf, sizeof(buf), "%04d/%02d/%02d %02d:%02d:%02d.%06d",
             tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
             tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, micro_seconds);
  } else {
    snprintf(buf, sizeof(buf), "%04d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
             tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
  }
  return buf;
}

// int main() {
//  Timestamp time;
//  std::cout << time.Now().ToFormattedString() << std::endl;
//  std::cout << time.Now().ToFormattedString(true) << std::endl;
//  return 0;
//}
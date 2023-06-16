#include "logging.h"

#include "current_thread.h"

namespace thread_info {
__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;
};  // namespace thread_info

const char* GetErrnoMsg(int saved_errno) {
  return ::strerror_r(saved_errno, thread_info::t_errnobuf,
                      sizeof(thread_info::t_errnobuf));
}

// 根据Level返回Level名字
const char* get_level_name[Logger::LogLevel::LEVEL_COUNT]{
    "TRACE ", "DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL ",
};

Logger::LogLevel InitLogLevel() { return Logger::INFO; }

Logger::LogLevel g_LogLevel = InitLogLevel();

static void DefaultOutput(const char* data, int len) {
  ::fwrite(data, len, sizeof(char), stdout);
}

static void DefaultFlush() { ::fflush(stdout); }

Logger::OutputFunc g_output = DefaultOutput;
Logger::FlushFunc g_flush = DefaultFlush;

Logger::Impl::Impl(Logger::LogLevel level, int saved_errno, const char* file,
                   int line)
    : time_(Timestamp::Now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file) {
  // 输出流 -> time
  FormatTime();
  // 写入日志等级
  stream_ << GeneralTemplate(get_level_name[level], 6);
  // TODO:error
  if (saved_errno != 0) {
    stream_ << GetErrnoMsg(saved_errno) << " (errno=" << saved_errno << ") ";
  }
}

// Timestamp::toString方法的思路，只不过这里需要输出到流
void Logger::Impl::FormatTime() {
  Timestamp now = Timestamp::Now();
  time_t seconds = static_cast<time_t>(now.micro_seconds_since_epoch() /
                                       Timestamp::kMicroSecondsPerSecond);
  int microseconds = static_cast<int>(now.micro_seconds_since_epoch() %
                                      Timestamp::kMicroSecondsPerSecond);

  struct tm* tm_time = ::localtime(&seconds);
  // 写入此线程存储的时间buf中
  snprintf(thread_info::t_time, sizeof(thread_info::t_time),
           "%4d/%02d/%02d %02d:%02d:%02d", tm_time->tm_year + 1900,
           tm_time->tm_mon + 1, tm_time->tm_mday, tm_time->tm_hour,
           tm_time->tm_min, tm_time->tm_sec);
  // 更新最后一次时间调用
  thread_info::t_lastSecond = seconds;

  // muduo使用Fmt格式化整数，这里我们直接写入buf
  char buf[32] = {0};
  snprintf(buf, sizeof(buf), "%06d ", microseconds);

  // 输出时间，附有微妙(之前是(buf, 6),少了一个空格)
  stream_ << GeneralTemplate(thread_info::t_time, 17)
          << GeneralTemplate(buf, 7);
}

void Logger::Impl::Finish() {
  stream_ << " - " << GeneralTemplate(basename_.data_, basename_.size_) << ':'
          << line_ << '\n';
}

// level默认为INFO等级
Logger::Logger(const char* file, int line) : impl_(INFO, 0, file, line) {}

Logger::Logger(const char* file, int line, Logger::LogLevel level)
    : impl_(level, 0, file, line) {}

// 可以打印调用函数
Logger::Logger(const char* file, int line, Logger::LogLevel level,
               const char* func)
    : impl_(level, 0, file, line) {
  impl_.stream_ << func << ' ';
}

Logger::~Logger() {
  impl_.Finish();
  // 获取buffer(stream_.buffer_)
  const LogStream::Buffer& buf(stream().buffer());
  // 输出(默认向终端输出)
  g_output(buf.data(), buf.Length());
  // FATAL情况终止程序
  if (impl_.level_ == FATAL) {
    g_flush();
    abort();
  }
}

void Logger::SetLogLevel(Logger::LogLevel level) { g_LogLevel = level; }

void Logger::SetOutput(OutputFunc out) { g_output = out; }

void Logger::SetFlush(FlushFunc flush) { g_flush = flush; }

#ifndef RYANLIB_LOGGER_LOGGING_H_
#define RYANLIB_LOGGER_LOGGING_H_

#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include <cstdio>
#include <functional>

#include "log_stream.h"
#include "timestamp.h"

// SourceFile的作用是提取文件名
class SourceFile {
 public:
  explicit SourceFile(const char* filename) : data_(filename) {
    // 找出data中出现/最后一次的位置，从而获取具体的文件名
    // 2022/10/26/test.log
    const char* slash = strrchr(filename, '/');
    if (slash) {
      data_ = slash + 1;
    }
    size_ = static_cast<int>(::strlen(data_));
  }

  const char* data_;
  int size_;
};

class Logger {
 public:
  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    LEVEL_COUNT,
  };

  // member function
  Logger(const char* file, int line);
  Logger(const char* file, int line, LogLevel level);
  Logger(const char* file, int line, LogLevel level, const char* func);
  ~Logger();

  // 流是会改变的
  LogStream& stream() { return impl_.stream_; }

  // TODO:static关键字作用的函数必须在源文件实现?
  static LogLevel logLevel();
  static void SetLogLevel(LogLevel level);

  // 输出函数和刷新缓冲区函数
  using OutputFunc = std::function<void(const char* msg, int len)>;
  using FlushFunc = std::function<void()>;
  static void SetOutput(OutputFunc);
  static void SetFlush(FlushFunc);

 private:
  // 内部类
  class Impl {
   public:
    using LogLevel = Logger::LogLevel;
    Impl(LogLevel level, int savedErrno, const char* file, int line);
    void FormatTime();
    void Finish();

    Timestamp time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile basename_;
  };

  // Logger's member variable
  Impl impl_;
};

extern Logger::LogLevel g_LogLevel;

inline Logger::LogLevel LogLevel() { return g_LogLevel; }

// 获取errno信息
const char* GetErrnoMsg(int saved_errno);

/**
 * 当日志等级小于对应等级才会输出
 * 比如设置等级为FATAL，则logLevel等级大于DEBUG和INFO，DEBUG和INFO等级的日志就不会输出
 */
#define LOG_DEBUG                  \
  if (LogLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO \
  if (LogLevel() <= Logger::INFO) Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()

#endif  // RYANLIB_LOGGER_LOGGING_H_
#ifndef RYANLIB_LOGGER_ASYNC_LOGGING_H_
#define RYANLIB_LOGGER_ASYNC_LOGGING_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include "fixed_buffer.h"
#include "log_file.h"
#include "log_stream.h"
#include "noncopyable.h"
#include "thread.h"

class AsyncLogging {
 public:
  AsyncLogging(const std::string& basename, off_t rollSize,
               int flushInterval = 3);
  ~AsyncLogging() {
    if (running_) {
      Stop();
    }
  }

  // 前端调用 append 写入日志
  void Append(const char* logling, int len);

  void Start() {
    running_ = true;
    thread_.Start();
  }

  void Stop() {
    running_ = false;
    cond_.notify_one();
    thread_.Join();
  }

 private:
  using Buffer = FixedBuffer<kLargeBuffer>;
  using BufferVector = std::vector<std::unique_ptr<Buffer>>;
  using BufferPtr = BufferVector::value_type;

  void ThreadFunc();

  const int flush_interval_;
  std::atomic<bool> running_;
  const std::string basename_;
  const off_t roll_size_;
  Thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;

  BufferPtr current_buffer_;
  BufferPtr next_buffer_;
  BufferVector buffers_;
};

#endif  // RYANLIB_LOGGER_ASYNC_LOGGING_H_
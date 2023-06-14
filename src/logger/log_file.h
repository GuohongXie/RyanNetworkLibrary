#ifndef RYANLIB_LOGGER_LOG_FILE_H_
#define RYANLIB_LOGGER_LOG_FILE_H_

#include <memory>
#include <mutex>

#include "file_util.h"

class LogFile {
 public:
  LogFile(const std::string& basename, off_t rool_size, int flush_interval = 3,
          int check_every_n = 1024);
  ~LogFile();

  void Append(const char* data, int len);
  void Flush();
  void RollFile();  //滚动日志

 private:
  static std::string GetLogFileName(const std::string& basename, time_t* now);
  void AppendInLock(const char* data, int len);

  const std::string basename_;
  const off_t roll_size_;
  const int flush_interval_;
  const int check_every_n_;

  int count_;

  std::unique_ptr<std::mutex> mutex_;
  time_t start_of_period_;
  time_t last_roll_;
  time_t last_flush_;
  std::unique_ptr<FileUtil> file_;

  const static int kPollPerSeconds_ = 60 * 60 * 24;
};

#endif  // RYANLIB_LOGGER_LOG_FILE_H_
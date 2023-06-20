#ifndef RYANLIB_LOGGER_FILE_UTIL_H_
#define RYANLIB_LOGGER_FILE_UTIL_H_

#include <stdio.h>
#include <string>

class FileUtil {
 public:
  explicit FileUtil(std::string& filename);
  ~FileUtil();
  void Append(const char* data, size_t len);
  void Flush();
  off_t written_bytes() const { return written_bytes_; }

 private:
  size_t Write(const char* data, size_t len);

  FILE* fp_;
  char buffer_[64 * 1024];
  off_t written_bytes_;
};

#endif  // RYANLIB_LOGGER_FILE_UTIL_H_
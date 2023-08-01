#include "logger/file_util.h"

#include "logger/logging.h"

FileUtil::FileUtil(std::string& fileName)
    : fp_(::fopen(fileName.c_str(), "ae")), written_bytes_(0) {
  // 将fd_缓冲区设置为本地的buffer_
  ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

FileUtil::~FileUtil() { ::fclose(fp_); }

void FileUtil::Append(const char* data, size_t len) {
  // 记录已经写入的数据大小
  size_t written = 0;

  while (written != len) {
    // 还需写入的数据大小
    size_t remain = len - written;
    size_t n = Write(data + written, remain);
    if (n != remain) {
      int err = ferror(fp_);
      if (err) {
        fprintf(stderr, "FileUtil::append() failed %s\n", GetErrnoMsg(err));
      }
    }
    // 更新写入的数据大小
    written += n;
  }
  // 记录目前为止写入的数据大小，超过限制会滚动日志
  written_bytes_ += written;
}

void FileUtil::Flush() { ::fflush(fp_); }

size_t FileUtil::Write(const char* data, size_t len) {
  /**
   * size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
   * -- buffer:指向数据块的指针
   * -- size:每个数据的大小，单位为Byte(例如：sizeof(int)就是4)
   * -- count:数据个数
   * -- stream:文件指针
   */
  return ::fwrite_unlocked(data, 1, len, fp_);
}
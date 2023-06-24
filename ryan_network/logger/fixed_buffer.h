#ifndef RYANLIB_LOGGER_FIXED_BUFFER_H_
#define RYANLIB_LOGGER_FIXED_BUFFER_H_

#include <cassert>
#include <cstring>
#include <strings.h>

#include <string>

#include "noncopyable.h"

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : Noncopyable {
 public:
  FixedBuffer() : curr_(data_) {}  //这个构造函数是什么鬼

  void Append(const char* buf, size_t len) {
    if (static_cast<size_t>(Avail()) > len) {
      memcpy(curr_, buf, len);
      curr_ += len;
    }
    // TODO:超出缓冲区长度如何处理，待补充
  }
  const char* data() const { return data_; }
  int Length() const { return static_cast<int>(end() - data_); }

  char* Current() { return curr_; } // why not const
  int Avail() const { return static_cast<int>(end() - curr_); }
  void Add(size_t len) { curr_ += len; }

  void Reset() { curr_ = data_; }
  void Bzero() { ::bzero(&data_, sizeof(data_)); }

  std::string ToString() const { return std::string(data_, Length()); }

 private:
  const char* end() const { return data_ + sizeof(data_); }

  char data_[SIZE];
  char* curr_;
};

#endif  // RYANLIB_LOGGER_FIXED_BUFFER_H_
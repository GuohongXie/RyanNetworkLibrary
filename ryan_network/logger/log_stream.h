#ifndef RYANLIB_LOGGER_LOG_STREAM_H_
#define RYANLIB_LOGGER_LOG_STREAM_H_

#include <string>

#include "buffer/fixed_buffer.h"
#include "base/noncopyable.h"

//比如SourceFile类和时间类就会用到
// const char* data_;
// int size_;

//陈硕认为以 operator<< 来输出数据非常适合logging
//因此写了一个LogStream class 
//代码不到300行，完全独立于iostream

//这个LogStream做到了类型安全和类型可扩展，效率也高。
//它不支持定制格式化、不支持Locale/facet,没有继承,buffer也没有继承和虚函数
//没有动态分配内存
//buffer 大小固定

//简单地说，适合logging以及简单地字符串转换

//这里的整数转换是陈硕自己写的，用的是Matthew Wilson的算法，这个算法比stdio和iostream都要快

//LogStream is not thread-safe, so its object should not be shared 
//over threads. The right way to use it is constructing a LogStream for each log message,
//since the cost of LogStream is so small, it will not cause performance loss

//其他程序库如何使用LogStream作为输出，可以使用模板的的方式实现，见陈硕的书的478页
//LogStream本身不支持格式化，不过我们很容易为他做扩展
//定义一个简单的fmt class就行了，而且不影响stream的状态


class GeneralTemplate : Noncopyable {
 public:
  GeneralTemplate() : data_(nullptr), len_(0) {}
  //此处为什么用explicit
  explicit GeneralTemplate(const char* data, int len)
      : data_(data), len_(len) {}

  const char* data_;
  int len_;
};

class LogStream : Noncopyable {
 public:
  using Buffer = FixedBuffer<kSmallBuffer>;

  void Append(const char* data, int len) { buffer_.Append(data, len); }
  const Buffer& buffer() const { return buffer_; }
  void ResetBuffer() { buffer_.Reset(); }

  //我们的LogStream需要重载运算符
  LogStream& operator<<(short);
  LogStream& operator<<(unsigned short);
  LogStream& operator<<(int);
  LogStream& operator<<(unsigned int);
  LogStream& operator<<(long);
  LogStream& operator<<(unsigned long);
  LogStream& operator<<(long long);
  LogStream& operator<<(unsigned long long);

  LogStream& operator<<(float v);
  LogStream& operator<<(double v);

  LogStream& operator<<(char c);
  LogStream& operator<<(const void* data);
  LogStream& operator<<(const char* str);
  LogStream& operator<<(const unsigned char* str);
  LogStream& operator<<(const std::string& str);
  LogStream& operator<<(const Buffer& buf);

  //(const char*, int)的重载
  LogStream& operator<<(const GeneralTemplate& g);

 private:
  static const int kMaxNumericSize = 48;
  //对于整型需要特殊处理
  template <typename T>
  void FormatInteger(T);

  Buffer buffer_;
};

#endif  // RYANLIB_LOGGER_LOG_STREAM_H_

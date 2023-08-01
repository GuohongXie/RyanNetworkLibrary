#include "logger/log_stream.h"

#include <algorithm>

static const char digits[] = {'9', '8', '7', '6', '5', '4', '3', '2', '1', '0',
                              '1', '2', '3', '4', '5', '6', '7', '8', '9'};

//整数到字符串的转换，可以视为itoa()的标准答案
//Matthew Wilson 的《 Efficient Integer to String Conversions 》系列文章 
//他的巧妙之处在千，用一个对称的 digits 数组搞定了负数转换的边界条件
//（ 二进制补码 的正负整数表示范围不对称）
template <typename T>
void LogStream::FormatInteger(T num) {
  if (buffer_.Avail() >= kMaxNumericSize) {
    char* start = buffer_.Current();
    char* cur = start;
    const char* zero = digits + 9;
    bool negative = (num < 0);  // 是否为负数

    // 末尾取值加入，最后反转
    do {
      int remainder = static_cast<int>(num % 10);
      *(cur++) = zero[remainder];
      num = num / 10;
    } while (num != 0);
    if (negative) {
      *(cur++) = '-';
    }
    *cur = '\0';
    std::reverse(start, cur);
    buffer_.Add(static_cast<int>(cur - start));  // cur_向后移动
  }
}

LogStream& LogStream::operator<<(short v) {
  *this << static_cast<int>(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(int v) {
  FormatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
  FormatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long v) {
  FormatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
  FormatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v) {
  FormatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
  FormatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(float v) {
  *this << static_cast<double>(v);
  return *this;
}

LogStream& LogStream::operator<<(double v) {
  if (buffer_.Avail() >= kMaxNumericSize) {
    char buf[32];
    int len = snprintf(buffer_.Current(), kMaxNumericSize, "%.12g", v);
    buffer_.Add(len);
    return *this;
  }
}

LogStream& LogStream::operator<<(char c) {
  buffer_.Append(&c, 1);
  return *this;
}

LogStream& LogStream::operator<<(const void* data) {
  *this << static_cast<const char*>(data);
  return *this;
}

LogStream& LogStream::operator<<(const char* str) {
  if (str) {
    buffer_.Append(str, strlen(str));
  } else {
    buffer_.Append("(null)", 6);
  }
  return *this;
}

LogStream& LogStream::operator<<(const unsigned char* str) {
  return operator<<(reinterpret_cast<const char*>(str));
}

LogStream& LogStream::operator<<(const std::string& str) {
  buffer_.Append(str.c_str(), str.size());
  return *this;
}

LogStream& LogStream::operator<<(const Buffer& buf) {
  *this << buf.ToString();
  return *this;
}

LogStream& LogStream::operator<<(const GeneralTemplate& g) {
  buffer_.Append(g.data_, g.len_);
  return *this;
}
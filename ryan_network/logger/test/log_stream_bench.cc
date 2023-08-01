#include <cstdio>

#include <sstream>

#include "logger/log_stream.h"
#include "base/timestamp.h"
#include <cinttypes>


const size_t N = 1000000;

//#pragma GCC diagnostic ignored "-Wold-style-cast"

template <typename T>
void BenchPrintf(const char* fmt) {
  char buf[32];
  Timestamp start(Timestamp::Now());
  for (size_t i = 0; i < N; ++i) snprintf(buf, sizeof buf, fmt, (T)(i));
  Timestamp end(Timestamp::Now());

  printf("BenchPrintf %f\n", TimeDifference(end, start));
}

template <typename T>
void BenchStringStream() {
  Timestamp start(Timestamp::Now());
  std::ostringstream os;

  for (size_t i = 0; i < N; ++i) {
    os << (T)(i);
    os.seekp(0, std::ios_base::beg);
  }
  Timestamp end(Timestamp::Now());

  printf("BenchStringStream %f\n", TimeDifference(end, start));
}

template <typename T>
void BenchLogStream() {
  Timestamp start(Timestamp::Now());
  LogStream os;
  for (size_t i = 0; i < N; ++i) {
    os << (T)(i);
    os.ResetBuffer();
  }
  Timestamp end(Timestamp::Now());

  printf("BenchLogStream %f\n", TimeDifference(end, start));
}

int main() {
  BenchPrintf<int>("%d");

  puts("int");
  BenchPrintf<int>("%d");
  BenchStringStream<int>();
  BenchLogStream<int>();

  puts("double");
  BenchPrintf<double>("%.12g");
  BenchStringStream<double>();
  BenchLogStream<double>();

  puts("int64_t");
  BenchPrintf<int64_t>("%" PRId64);
  BenchStringStream<int64_t>();
  BenchLogStream<int64_t>();

  puts("void*");
  BenchPrintf<void*>("%p");
  BenchStringStream<void*>();
  BenchLogStream<void*>();
}
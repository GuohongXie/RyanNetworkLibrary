#include "logger/logging.h"

#include <cstdio>

#include <unistd.h>

#include "logger/log_file.h"
#include "base/thread_pool.h"

int g_total;
FILE* g_file;
std::unique_ptr<LogFile> g_logFile;

void DummyOutput(const char* msg, int len) {
  g_total += len;
  if (g_file) {
    fwrite(msg, 1, len, g_file);
  } else if (g_logFile) {
    g_logFile->Append(msg, len);
  }
}

void bench(const char* type) {
  Logger::SetOutput(DummyOutput);
  Timestamp start(Timestamp::Now());
  g_total = 0;

  int n = 1000 * 1000;
  const bool kLongLog = false;
  std::string empty = " ";
  std::string longStr(3000, 'X');
  longStr += " ";
  for (int i = 0; i < n; ++i) {
    LOG_INFO << "Hello 0123456789"
             << " abcdefghijklmnopqrstuvwxyz" << (kLongLog ? longStr : empty)
             << i;
  }
  Timestamp end(Timestamp::Now());
  double seconds = TimeDifference(end, start);
  printf("%12s: %f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n", type,
         seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));
}

void LogInThread() {
  LOG_INFO << "LogInThread";
  ::usleep(1000);
}

int main() {
  ::getppid();  // for ltrace and strace

  ThreadPool pool("pool");
  pool.SetThreadSize(5);
  pool.Add(LogInThread);
  pool.Add(LogInThread);
  pool.Add(LogInThread);
  pool.Add(LogInThread);
  pool.Add(LogInThread);

  LOG_TRACE << "trace";
  LOG_DEBUG << "debug";
  LOG_INFO << "Hello";
  LOG_WARN << "World";
  LOG_ERROR << "Error";
  LOG_INFO << sizeof(Logger);
  LOG_INFO << sizeof(LogStream);
  LOG_INFO << sizeof(LogStream::Buffer);

  sleep(1);
  bench("nop");

  char buffer[64 * 1024];

  g_file = fopen("/dev/null", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench("/dev/null");
  fclose(g_file);

  g_file = fopen("/tmp/log", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench("/tmp/log");
  fclose(g_file);

  g_file = nullptr;
  g_logFile.reset(new LogFile("test_log_st", 500 * 1000 * 1000, false));
  bench("test_log_st");

  g_logFile.reset(new LogFile("test_log_mt", 500 * 1000 * 1000, true));
  bench("test_log_mt");
  g_logFile.reset();

  
}

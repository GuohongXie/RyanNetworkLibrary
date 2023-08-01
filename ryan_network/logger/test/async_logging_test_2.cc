#include "logger/async_logging.h"

#include <cstdio>
#include <string>

#include <sys/resource.h>
#include <unistd.h>

#include "logger/logging.h"
#include "base/timestamp.h"

off_t kRollSize = 500 * 1000 * 1000;

AsyncLogging* g_asyncLog = nullptr;

void AsyncOutput(const char* msg, int len) { g_asyncLog->Append(msg, len); }

void Bench(bool longLog) {
  Logger::SetOutput(AsyncOutput);

  int cnt = 0;
  const int kBatch = 1000;
  std::string empty = " ";
  std::string longStr(3000, 'X');
  longStr += " ";

  for (int t = 0; t < 30; ++t) {
    Timestamp start = Timestamp::Now();
    for (int i = 0; i < kBatch; ++i) {
      LOG_INFO << "Hello 0123456789"
               << " abcdefghijklmnopqrstuvwxyz " << (longLog ? longStr : empty)
               << cnt;
      ++cnt;
    }
    Timestamp end = Timestamp::Now();
    printf("%f\n", TimeDifference(end, start) * 1000000 / kBatch);
    struct timespec ts = {0, 500 * 1000 * 1000};
    ::nanosleep(&ts, nullptr);
  }
}

int main(int argc, char* argv[]) {
  {
    // set max virtual memory to 2GB.
    size_t kOneGB = 1000 * 1024 * 1024;
    rlimit rl = {2 * kOneGB, 2 * kOneGB};
    setrlimit(RLIMIT_AS, &rl);
  }

  printf("pid = %d\n", ::getpid());

  char name[256] = {'\0'};
  strncpy(name, argv[0], sizeof(name) - 1);
  AsyncLogging log(::basename(name), kRollSize);
  log.Start();
  g_asyncLog = &log;

  bool longLog = argc > 1;
  Bench(longLog);
}

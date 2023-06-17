#include "file_util.h"

#include <stdio.h>
#include <string>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>


int main()
{
  std::string result;
  int64_t size = 0;
  int err = FileUtil::ReadFile("/proc/self", 1024, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::ReadFile("/proc/self", 1024, &result, NULL);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::ReadFile("/proc/self/cmdline", 1024, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::ReadFile("/dev/null", 1024, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::ReadFile("/dev/zero", 1024, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::ReadFile("/notexist", 1024, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::ReadFile("/dev/zero", 102400, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::ReadFile("/dev/zero", 102400, &result, NULL);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
}


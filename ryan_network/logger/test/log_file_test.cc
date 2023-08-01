#include "logger/log_file.h"

#include <unistd.h>

#include "logger/logging.h"

std::unique_ptr<LogFile> g_logFile;

void OutputFunc(const char* msg, int len) { g_logFile->Append(msg, len); }

void FlushFunc() { g_logFile->Flush(); }

int main(int argc, char* argv[]) {
  char name[256] = {'\0'};
  ::strncpy(name, argv[0], sizeof(name - 1));
  g_logFile.reset(new LogFile(::basename(name), 200 * 1000));
  Logger::SetOutput(OutputFunc);
  Logger::SetFlush(FlushFunc);

  std::string line =
      "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

  for (int i = 0; i < 10000; ++i) {
    LOG_INFO << line << i;

    ::usleep(1000);
  }
}

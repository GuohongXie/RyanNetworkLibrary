#include "base/current_thread.h"

namespace current_thread {
thread_local int t_cached_tid = 0;

void CacheTid() {
  if (t_cached_tid == 0) {
    t_cached_tid = static_cast<pid_t>(::syscall(SYS_gettid));
  }
}

bool IsMainThread() { return Tid() == ::getpid(); }

}  // namespace current_thread
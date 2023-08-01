#ifndef RYANLIB_BASE_CURRENT_THREAD_H_
#define RYANLIB_BASE_CURRENT_THREAD_H_

#include <cstdint>

#include <sys/syscall.h>
#include <unistd.h>

namespace current_thread {
extern thread_local int t_cached_tid;  //保存tid缓冲，避免多次系统调用
void CacheTid();

//内联函数
inline int Tid() {
  if (__builtin_expect(t_cached_tid == 0, 0)) {
    CacheTid();
  }
  return t_cached_tid;
}
}  // namespace current_thread

#endif  // RYANLIB_BASE_CURRENT_THREAD_H_
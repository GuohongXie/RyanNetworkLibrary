#include <stdlib.h>

#include "epoll_poller.h"
#include "poll_poller.h"
#include "poller.h"
// 获取默认的Poller实现方式
Poller* Poller::NewDefaultPoller(EventLoop* loop) {
  if (::getenv("MUDUO_USE_POLL")) {
    return new PollPoller(loop);  // 生成poll实例
  } else {
    return new EpollPoller(loop);  // 生成epoll实例
  }
}

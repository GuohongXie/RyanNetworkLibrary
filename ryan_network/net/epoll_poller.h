#ifndef RYANLIB_NET_EPOLL_POLLER_H_
#define RYANLIB_NET_EPOLL_POLLER_H_

#include <sys/epoll.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>

#include <vector>

#include "logging.h"
#include "poller.h"
#include "timestamp.h"

/**
 * epoll_create
 * epoll_ctl
 * epoll_wait
 */
class EpollPoller : public Poller {
  using EventList = std::vector<epoll_event>;

 public:
  EpollPoller(EventLoop* Loop);
  ~EpollPoller() override;

  // 重写基类Poller的抽象方法
  Timestamp Poll(int timeoutMs, ChannelList* activeChannels) override;
  void UpdateChannel(Channel* channel) override;
  void RemoveChannel(Channel* channel) override;

 private:
  // 默认监听事件数量
  static const int kInitEventListSize = 16;

  // 填写活跃的连接
  void FillActiveChannels(int numEvents, ChannelList* activeChannels) const;
  // 更新channel通道，本质是调用了epoll_ctl
  void Update(int operation, Channel* channel);

  int epollfd_;  // epoll_create在内核创建空间返回的fd
  EventList events_;  // 用于存放epoll_wait返回的所有发生的事件的文件描述符
};

#endif  // RYANLIB_NET_EPOLL_POLLER_H_
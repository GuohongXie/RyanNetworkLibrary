#ifndef RYANLIB_NET_POLLER_H_
#define RYANLIB_NET_POLLER_H_

#include <unordered_map>
#include <vector>

#include "channel.h"
#include "timestamp.h"
#include "noncopyable.h"

// muduo库中多路事件分发器的核心IO复用模块
class Poller : Noncopyable {
 public:
  using ChannelList = std::vector<Channel*>;

  Poller(EventLoop* Loop);
  virtual ~Poller() = default;

  // 需要交给派生类实现的接口
  virtual Timestamp Poll(int timeoutMs, ChannelList* activeChannels) = 0;
  virtual void UpdateChannel(Channel* channel) = 0;
  virtual void RemoveChannel(Channel* channel) = 0;

  // 判断 channel是否注册到 poller当中
  bool HasChannel(Channel* channel) const;

  // EventLoop可以通过该接口获取默认的IO复用实现方式(默认epoll)
  /**
   * 它的实现并不在 Poller.cc 文件中
   * 如果要实现则可以预料会其会包含EPollPoller PollPoller
   * 那么外面就会在基类引用派生类的头文件，这个抽象的设计就不好
   * 所以外面会单独创建一个 DefaultPoller.cc 的文件去实现
   */
  static Poller* NewDefaultPoller(EventLoop* Loop);

 protected:
  using ChannelMap = std::unordered_map<int, Channel*>;
  // 储存 channel 的映射，（sockfd -> channel*）
  ChannelMap channels_;

 private:
  EventLoop* owner_loop_;  // 定义Poller所属的事件循环EventLoop
};

#endif  // RYANLIB_NET_POLLER_H_
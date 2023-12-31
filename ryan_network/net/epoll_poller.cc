#include "net/epoll_poller.h"


// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

namespace {
const int kNew = -1;     // 某个channel还没添加至Poller channel的成员index_初始化为-1
const int kAdded = 1;    // 某个channel已经添加至Poller
const int kDeleted = 2;  // 某个channel已经从Poller删除
}  // namespace

// TODO:epoll_create1(EPOLL_CLOEXEC)
EpollPoller::EpollPoller(EventLoop* loop)
    : Poller(loop),  // 传给基类
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG_FATAL << "epoll_create() error:" << errno;
  }
}

EpollPoller::~EpollPoller() { ::close(epollfd_); }

Timestamp EpollPoller::Poll(int timeoutMs, ChannelList* active_channels) {
  // 高并发情况经常被调用，影响效率，使用debug模式可以手动关闭

  size_t num_events = ::epoll_wait(epollfd_, &(*events_.begin()),
                                   static_cast<int>(events_.size()), timeoutMs);
  int saveErrno = errno;
  Timestamp now(Timestamp::Now());

  // 有事件产生
  if (num_events > 0) {
    FillActiveChannels(num_events, active_channels);  // 填充活跃的channels
    // 对events_进行扩容操作
    if (num_events == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  }
  // 超时
  else if (num_events == 0) {
    LOG_DEBUG << "timeout!";
  }
  // 出错
  else {
    // 不是终端错误
    if (saveErrno != EINTR) {
      errno = saveErrno;
      LOG_ERROR << "EPollPoller::poll() failed";
    }
  }
  return now;
}

/**
 * Channel::Update => EventLoop::updateChannel => Poller::updateChannel
 * Channel::remove => EventLoop::RemoveChannel => Poller::RemoveChannel
 */
void EpollPoller::UpdateChannel(Channel* channel) {
  Poller::AssertInLoopThread();
  // TODO:__FUNCTION__
  // 获取参数channel在epoll的状态
  const int index = channel->index();

  // 未添加状态和已删除状态都有可能会被再次添加到epoll中
  if (index == kNew || index == kDeleted) {
      int fd = channel->fd();
    // 添加到键值对
    if (index == kNew) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    } else { // index == kAdd
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }
    // 修改channel的状态，此时是已添加状态
    channel->set_index(kAdded);
    // 向epoll对象加入channel
    Update(EPOLL_CTL_ADD, channel);
  }
  // channel已经在poller上注册过
  else {
    // update existing one with EPOLL_CTL_MOD/DEL
    int fd = channel->fd();
    (void)fd;  //作用是？
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == kAdded);

    // 没有感兴趣事件说明可以从epoll对象中删除该channel了
    if (channel->IsNoneEvent()) {
      Update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    }
    // 还有事件说明之前的事件删除，但是被修改了
    else {
      Update(EPOLL_CTL_MOD, channel);
    }
  }
}

// 填写活跃的连接
void EpollPoller::FillActiveChannels(int num_events,
                                     ChannelList* active_channels) const {
  for (int i = 0; i < num_events; ++i) {
    // void* => Channel*
    Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
    channel->set_revents(events_[i].events);
    active_channels->push_back(channel);
  }
}

void EpollPoller::RemoveChannel(Channel* channel) {
  Poller::AssertInLoopThread();
  // 从Map中删除
  int fd = channel->fd();
  //LOG_TRACE << "fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->IsNoneEvent());

  int index = channel->index();
  assert(index == kAdded || index == kDeleted);
  size_t n = channels_.erase(fd);
  (void)n;  //这一行的作用是什么？
  assert(n == 1);

  if (index == kAdded) {
    // 如果此fd已经被添加到Poller中，则还需从epoll对象中删除
    Update(EPOLL_CTL_DEL, channel);
  }
  // 重新设置channel的状态为未被Poller注册
  channel->set_index(kNew);
}

void EpollPoller::Update(int operation, Channel* channel) {
  epoll_event event;
  ::memset(&event, 0, sizeof(event));

  int fd = channel->fd();
  event.events = channel->events();
  event.data.fd = fd;
  event.data.ptr = channel;

  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_ERROR << "epoll_ctl() del error:" << errno;
    } else {
      LOG_FATAL << "epoll_ctl add/mod error:" << errno;
    }
  }
}
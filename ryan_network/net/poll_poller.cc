#include "poll_poller.h"

#include <cassert>
#include <errno.h>
#include <poll.h>

#include "logging.h"
#include "channel.h"


PollPoller::PollPoller(EventLoop* loop) : Poller(loop) {}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::Poll(int timeoutMs, ChannelList* activeChannels) {
  // XXX poll_fds_ shouldn't change
  int num_events = ::poll(&*poll_fds_.begin(), poll_fds_.size(), timeoutMs);
  int saved_errno = errno;
  Timestamp now(Timestamp::Now());
  if (num_events > 0) {
    LOG_TRACE << num_events << " events happened";
    FillActiveChannels(num_events, activeChannels);
  } else if (num_events == 0) {
    //LOG_TRACE << " nothing happened";
  } else {
    if (saved_errno != EINTR) {
      errno = saved_errno;
      //LOG_SYSERR << "PollPoller::Poll()";
    }
  }
  return now;
}

void PollPoller::FillActiveChannels(int num_events,
                                    ChannelList* activeChannels) const {
  for (PollFdList::const_iterator pfd = poll_fds_.begin();
       pfd != poll_fds_.end() && num_events > 0; ++pfd) {
    if (pfd->revents > 0) {
      --num_events;
      ChannelMap::const_iterator ch = channels_.find(pfd->fd);
      assert(ch != channels_.end());
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->set_revents(pfd->revents);
      // pfd->revents = 0;
      activeChannels->push_back(channel);
    }
  }
}

void PollPoller::UpdateChannel(Channel* channel) {
  Poller::AssertInLoopThread();
  //LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->index() < 0) {
    // a new one, add to poll_fds_
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    poll_fds_.push_back(pfd);
    int idx = static_cast<int>(poll_fds_.size()) - 1;
    channel->set_index(idx);
    channels_[pfd.fd] = channel;
  } else {
    // update existing one
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(poll_fds_.size()));
    struct pollfd& pfd = poll_fds_[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    if (channel->IsNoneEvent()) {
      // ignore this pollfd
      pfd.fd = -channel->fd() - 1;
    }
  }
}

void PollPoller::RemoveChannel(Channel* channel) {
  Poller::AssertInLoopThread();
  //LOG_TRACE << "fd = " << channel->fd();
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  assert(channel->isNoneEvent());
  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(poll_fds_.size()));
  const struct pollfd& pfd = poll_fds_[idx];
  (void)pfd; // 这一行的意思是将pfd变量的值转换成void类型，以便于编译器不会报错
  assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
  size_t n = channels_.erase(channel->fd());
  assert(n == 1);
  (void)n;
  if (static_cast<size_t>(idx) == poll_fds_.size() - 1) {
    poll_fds_.pop_back();
  } else {
    int channelAtEnd = poll_fds_.back().fd;
    iter_swap(poll_fds_.begin() + idx, poll_fds_.end() - 1);
    if (channelAtEnd < 0) {
      channelAtEnd = -channelAtEnd - 1;
    }
    channels_[channelAtEnd]->set_index(idx);
    poll_fds_.pop_back();
  }
}
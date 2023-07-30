// This is an internal header file, you should not include this.

#ifndef RYANLIB_NET_POLL_POLLER_H_
#define RYANLIB_NET_POLL_POLLER_H_

#include <vector>

#include "poller.h"

struct pollfd;

///
/// IO Multiplexing with poll(2).
///
class PollPoller : public Poller {
 public:
  PollPoller(EventLoop* loop);
  ~PollPoller() override;

  Timestamp Poll(int timeoutMs, ChannelList* activeChannels) override;
  void UpdateChannel(Channel* channel) override;
  void RemoveChannel(Channel* channel) override;

 private:
  void FillActiveChannels(int numEvents, ChannelList* activeChannels) const;

  typedef std::vector<struct pollfd> PollFdList;
  PollFdList poll_fds_;
};

#endif  // RYANLIB_NET_POLL_POLLER_H_

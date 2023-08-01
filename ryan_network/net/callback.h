#ifndef RYANLIB_NET_CALLBACK_H_
#define RYANLIB_NET_CALLBACK_H_

#include <functional>
#include <memory>

class Buffer;
class TcpConnection;
class Timestamp;

using TimerCallback = std::function<void()>;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWatermarkCallback =
    std::function<void(const TcpConnectionPtr&, size_t)>;

using MessageCallback =
    std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;

#endif  // RYANLIB_NET_CALLBACK_H_
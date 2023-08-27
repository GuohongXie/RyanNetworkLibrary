#ifndef RYANLIB_TCP_CONNECTION_TCP_SERVER_H_
#define RYANLIB_TCP_CONNECTION_TCP_SERVER_H_

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "net/acceptor.h"
#include "net/callback.h"
#include "net/event_loop.h"
#include "net/event_loop_thread_pool.h"
#include "net/inet_address.h"
#include "tcp_connection/tcp_connection.h"
#include "base/noncopyable.h"

/**
 * 我们用户编写的时候就是使用的TcpServer
 * 我们向里面注册各种回调函数
 */
class TcpServer : Noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  enum Option {
    kNoReusePort,
    kReusePort,
  };

  TcpServer(EventLoop* loop, const InetAddress& ListenAddr,
            const std::string& nameArg, Option option = kNoReusePort);
  ~TcpServer();

  // 设置回调函数(用户自定义的函数传入)
  void SetThreadInitCallback(const ThreadInitCallback& cb) {
    thread_init_callback_ = cb;
  }
  void SetConnectionCallback(const ConnectionCallback& cb) {
    connection_callback_ = cb;
  }
  void SetMessageCallback(const MessageCallback& cb) { message_callback_ = cb; }
  void SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
    write_complete_callback_ = cb;
  }

  // 设置底层subLoop的个数
  void SetThreadNum(int num_threads);

  // 开启服务器监听
  void Start();

  EventLoop* GetLoop() const { return loop_; }

  const std::string name() { return name_; }

  const std::string ip_port() { return ip_port_; }

 private:
  void NewConnection(int sockfd, const InetAddress& peerAddr);
  void RemoveConnection(const TcpConnectionPtr& conn);
  void RemoveConnectionInLoop(const TcpConnectionPtr& conn);

  /**
   * key:     std::string
   * value:   std::shared_ptr<TcpConnection>
   */
  using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

  EventLoop* loop_;                     // 用户定义的baseLoop
  const std::string ip_port_;           // 传入的IP地址和端口号
  const std::string name_;              // TcpServer名字
  std::unique_ptr<Acceptor> acceptor_;  // Acceptor对象负责监视

  std::shared_ptr<EventLoopThreadPool> thread_pool_;  // 线程池

  ConnectionCallback connection_callback_;  // 有新连接时的回调函数
  MessageCallback message_callback_;        // 有读写消息时的回调函数
  WriteCompleteCallback write_complete_callback_;  // 消息发送完成以后的回调函数

  ThreadInitCallback thread_init_callback_;  // loop线程初始化的回调函数
  std::atomic_int started_;                  // TcpServer

  int next_conn_id_;           // 连接索引
  ConnectionMap connections_;  // 保存所有的连接
};

#endif  // RYANLIB_TCP_CONNECTION_TCP_SERVER_H_
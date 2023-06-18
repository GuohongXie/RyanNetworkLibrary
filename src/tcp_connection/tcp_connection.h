#ifndef RYANLIB_TCP_CONNECTION_TCP_CONNECTION_H_
#define RYANLIB_TCP_CONNECTION_TCP_CONNECTION_H_

#include <atomic>
#include <memory>
#include <string>

#include "buffer.h"
#include "callback.h"
#include "inet_address.h"
#include "timestamp.h"
#include "noncopyable.h"

class Channel;
class EventLoop;
class Socket;

class TcpConnection : public Noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop, const std::string& nameArg, int sockfd,
                const InetAddress& localAddr, const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* GetLoop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& LocalAddress() const { return local_addr_; }
  const InetAddress& PeerAddress() const { return peer_addr_; }

  bool Connected() const { return state_ == kConnected; }

  // 发送数据
  void Send(const std::string& buf);
  void Send(Buffer* buf);

  // 关闭连接
  void Shutdown();

  // 保存用户自定义的回调函数
  void SetConnectionCallback(const ConnectionCallback& cb) {
    connection_callback_ = cb;
  }
  void SetMessageCallback(const MessageCallback& cb) { message_callback_ = cb; }
  void SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
    write_complete_callback_ = cb;
  }
  void SetCloseCallback(const CloseCallback& cb) { close_callback_ = cb; }
  void SetHighWatermarkCallback(const HighWaterMarkCallback& cb,
                                size_t highWaterMark) {
    high_watermark_callback_ = cb;
    high_watermark_ = highWaterMark;
  }

  // TcpServer会调用
  void ConnectEstablished();  // 连接建立
  void ConnectDestroyed();    // 连接销毁

 private:
  enum StateE {
    kDisconnected,  // 已经断开连接
    kConnecting,    // 正在连接
    kConnected,     // 已连接
    kDisconnecting  // 正在断开连接
  };
  void setState(StateE state) { state_ = state; }

  // 注册到channel上的回调函数，poller通知后会调用这些函数处理
  // 然后这些函数最后会再调用从用户那里传来的回调函数
  void HandleRead(Timestamp receiveTime);
  void HandleWrite();
  void HandleClose();
  void HandleError();

  void SendInLoop(const void* message, size_t len);
  void SendInLoop(const std::string& message);
  void ShutdownInLoop();

  EventLoop* loop_;  // 属于哪个subLoop（如果是单线程则为mainLoop）
  const std::string name_;
  std::atomic_int state_;  // 连接状态
  bool reading_;

  std::unique_ptr<Socket> socket_;
  ;
  std::unique_ptr<Channel> channel_;

  const InetAddress local_addr_;  // 本服务器地址
  const InetAddress peer_addr_;   // 对端地址

  /**
   * 用户自定义的这些事件的处理函数，然后传递给 TcpServer
   * TcpServer 再在创建 TcpConnection 对象时候设置这些回调函数到 TcpConnection中
   */
  ConnectionCallback connection_callback_;        // 有新连接时的回调
  MessageCallback message_callback_;              // 有读写消息时的回调
  WriteCompleteCallback write_complete_callback_;  // 消息发送完成以后的回调
  CloseCallback close_callback_;  // 客户端关闭连接的回调
  HighWaterMarkCallback high_watermark_callback_;  // 超出水位实现的回调
  size_t high_watermark_;

  Buffer input_buffer_;   // 读取数据的缓冲区
  Buffer output_buffer_;  // 发送数据的缓冲区
};

#endif  // RYANLIB_TCP_CONNECTION_TCP_CONNECTION_H_
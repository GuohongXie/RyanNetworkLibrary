#include "tcp_connection/tcp_server.h"

#include <cstring>

#include <functional>

#include "logger/logging.h"
#include "tcp_connection/tcp_connection.h"

// 检查传入的 baseLoop 指针是否有意义
static EventLoop* CheckLoopNotNull(EventLoop* loop) {
  if (loop == nullptr) {
    LOG_FATAL << "mainLoop is null!";
  }
  return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr,
                     const std::string& nameArg, Option option)
    : loop_(CheckLoopNotNull(loop)),
      ip_port_(listenAddr.ToIpPort()),
      name_(nameArg),
      acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      thread_pool_(new EventLoopThreadPool(loop, name_)),
      connection_callback_(DefaultConnectionCallback),
      message_callback_(DefaultMessageCallback),
      write_complete_callback_(),
      thread_init_callback_(),
      started_(0),
      next_conn_id_(1) {
  // 当有新用户连接时，Acceptor类中绑定的acceptChannel_会有读事件发生执行handleRead()调用TcpServer::newConnection回调
  acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this,
                                                std::placeholders::_1,
                                                std::placeholders::_2));
}

TcpServer::~TcpServer() {
  for (auto& item : connections_) {
    TcpConnectionPtr conn(item.second);
    // 把原始的智能指针复位 让栈空间的TcpConnectionPtr conn指向该对象
    // 当conn出了其作用域 即可释放智能指针指向的对象
    item.second.reset();
    // 销毁连接
    conn->GetLoop()->RunInLoop(
        std::bind(&TcpConnection::ConnectDestroyed, conn));
  }
}

// 设置底层subloop的个数
void TcpServer::SetThreadNum(int num_threads) {
  thread_pool_->SetThreadNum(num_threads);
}

// 开启服务器监听
void TcpServer::Start() {
  if (started_++ == 0) {
    // 启动底层的lopp线程池
    thread_pool_->Start(thread_init_callback_);
    // acceptor_.get()绑定时候需要地址
    loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
  }
}

// 有一个新用户连接，acceptor会执行这个回调操作，负责将mainLoop接收到的请求连接(acceptChannel_会有读事件发生)通过回调轮询分发给subLoop去处理
void TcpServer::NewConnection(int sockfd, const InetAddress& peer_addr) {
  // 轮询算法 选择一个subLoop 来管理connfd对应的channel
  EventLoop* io_loop = thread_pool_->GetNextLoop();
  // 提示信息
  char buf[64] = {0};
  snprintf(buf, sizeof buf, "-%s#%d", ip_port_.c_str(), next_conn_id_);
  // 这里没有设置为原子类是因为其只在mainloop中执行 不涉及线程安全问题
  ++next_conn_id_;
  // 新连接名字
  std::string conn_name = name_ + buf;

  LOG_INFO << "TcpServer::NewConnection [" << name_.c_str()
           << "] - new connection [" << conn_name.c_str() << "] from "
           << peer_addr.ToIpPort().c_str();

  // 通过sockfd获取其绑定的本机的ip地址和端口信息
  sockaddr_in local;
  ::memset(&local, 0, sizeof(local));
  socklen_t addrlen = sizeof(local);
  if (::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0) {
    LOG_ERROR << "sockets::getLocalAddr() failed";
  }

  InetAddress local_addr(local);
  TcpConnectionPtr conn(
      new TcpConnection(io_loop, conn_name, sockfd, local_addr, peer_addr));
  connections_[conn_name] = conn;
  // 下面的回调都是用户设置给TcpServer =>
  // TcpConnection的，至于Channel绑定的则是TcpConnection设置的四个，handleRead,handleWrite...
  // 这下面的回调用于handlexxx函数中
  conn->SetConnectionCallback(connection_callback_);
  conn->SetMessageCallback(message_callback_);
  conn->SetWriteCompleteCallback(write_complete_callback_);

  // 设置了如何关闭连接的回调
  conn->SetCloseCallback(
      std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));

  io_loop->RunInLoop(std::bind(&TcpConnection::ConnectEstablished, conn));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
  loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr& conn) {
  LOG_INFO << "TcpServer::RemoveConnectionInLoop [" << name_.c_str()
           << "] - connection " << conn->name().c_str();

  connections_.erase(conn->name());
  EventLoop* io_loop = conn->GetLoop();
  io_loop->QueueInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
}
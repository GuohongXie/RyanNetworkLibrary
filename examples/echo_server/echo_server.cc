#include "net/event_loop.h"
#include "tcp_connection/tcp_server.h"
#include "logger/logging.h"
#include "logger/async_logging.h"

class EchoServer {
 public:
  EchoServer(EventLoop* loop, const InetAddress& addr, const std::string& name)
      : server_(loop, addr, name), loop_(loop) {
    // 注册回调函数
    server_.SetConnectionCallback(
        std::bind(&EchoServer::OnConnection, this, std::placeholders::_1));

    server_.SetMessageCallback(
        std::bind(&EchoServer::OnMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));

    // 设置合适的subloop线程数量
    // server_.setThreadNum(3);
  }

  void Start() { server_.Start(); }

 private:
  // 连接建立或断开的回调函数
  void OnConnection(const TcpConnectionPtr& conn) {
    if (conn->Connected()) {
      LOG_INFO << "Connection UP : " << conn->PeerAddress().ToIpPort().c_str();
    } else {
      LOG_INFO << "Connection DOWN : "
               << conn->PeerAddress().ToIpPort().c_str();
    }
  }

  // 可读写事件回调
  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    std::string msg = buf->RetrieveAllAsString();
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
             << "data received at " << time.ToFormattedString();
    conn->Send(msg);
    // conn->shutdown();   // 关闭写端 底层响应EPOLLHUP => 执行closeCallback_
  }

  EventLoop* loop_;
  TcpServer server_;
};

int main() {
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress addr(18080);
  EchoServer server(&loop, addr, "EchoServer");
  server.Start();
  loop.Loop();

  return 0;
}
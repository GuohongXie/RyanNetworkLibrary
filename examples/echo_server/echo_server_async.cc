#include "async_logging.h"
#include "event_loop.h"
#include "logging.h"
#include "tcp_server.h"

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
      // LOG_INFO("Connection UP : %s", conn->PeerAddress().ToIpPort().c_str());
    } else {
      LOG_INFO << "Connection DOWN : "
               << conn->PeerAddress().ToIpPort().c_str();
    }
  }

  // 可读写事件回调
  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    printf("OnMessage!\n");
    std::string msg = buf->RetrieveAllAsString();
    conn->Send(msg);
    // conn->shutdown();   // 关闭写端 底层响应EPOLLHUP => 执行closeCallback_
  }

  EventLoop* loop_;
  TcpServer server_;
};

int kRollSize = 500 * 1000 * 1000;

// 异步日志
std::unique_ptr<AsyncLogging> g_asyncLog;

void AsyncOutput(const char* msg, int len) { g_asyncLog->Append(msg, len); }

void SetLogging(const char* argv0) {
  Logger::SetOutput(AsyncOutput);
  char name[256];
  strncpy(name, argv0, 256);
  g_asyncLog.reset(new AsyncLogging(::basename(name), kRollSize));
  g_asyncLog->Start();
}

int main(int argc, char* argv[]) {
  SetLogging(argv[0]);

  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress addr(18080);
  EchoServer server(&loop, addr, "EchoServer");
  server.Start();
  loop.Loop();

  return 0;
}
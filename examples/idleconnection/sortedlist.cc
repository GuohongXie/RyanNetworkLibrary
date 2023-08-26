#include <cstdio>
#include <functional>
#include <list>

#include <unistd.h>

#include "logger/logging.h"
#include "net/event_loop.h"
#include "tcp_connection/tcp_server.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

// RFC 862
class EchoServer {
 public:
  EchoServer(EventLoop* loop, const InetAddress& listenAddr, int idle_seconds);

  void Start() { server_.Start(); }

 private:
  void OnConnection(const TcpConnectionPtr& conn);

  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);

  void OnTimer();

  void DumpConnectionList() const;

  using WeakTcpConnectionPtr = std::weak_ptr<TcpConnection>;
  using WeakConnectionList = std::list<WeakTcpConnectionPtr>;

  struct Node {
    Timestamp lastReceiveTime;
    WeakConnectionList::iterator position;
  };

  TcpServer server_;
  int idleSeconds_;
  WeakConnectionList connectionList_;
};

EchoServer::EchoServer(EventLoop* loop, const InetAddress& listenAddr,
                       int idle_seconds)
    : server_(loop, listenAddr, "EchoServer"), idleSeconds_(idle_seconds) {
  server_.SetConnectionCallback(std::bind(&EchoServer::OnConnection, this, _1));
  server_.SetMessageCallback(
      std::bind(&EchoServer::OnMessage, this, _1, _2, _3));
  loop->RunEvery(1.0, std::bind(&EchoServer::OnTimer, this));
  DumpConnectionList();
}

void EchoServer::OnConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "EchoServer - " << conn->PeerAddress().ToIpPort() << " -> "
           << conn->LocalAddress().ToIpPort() << " is "
           << (conn->Connected() ? "UP" : "DOWN");

  if (conn->Connected()) {
    Node node;
    node.lastReceiveTime = Timestamp::Now();
    connectionList_.push_back(conn);
    node.position = --connectionList_.end();
    conn->SetContext(node);
  } else {
    assert(conn->GetContext().has_value());
    const Node& node = std::any_cast<const Node&>(conn->GetContext());
    connectionList_.erase(node.position);
  }
  DumpConnectionList();
}

void EchoServer::OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
                           Timestamp time) {
  std::string msg(buf->RetrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at "
           << time.ToString();
  conn->Send(msg);

  assert(conn->GetContext().has_value());
  Node* node = std::any_cast<Node>(conn->GetMutableContext());
  node->lastReceiveTime = time;
  connectionList_.splice(connectionList_.end(), connectionList_,
                         node->position);
  assert(node->position == --connectionList_.end());

  DumpConnectionList();
}

void EchoServer::OnTimer() {
  DumpConnectionList();
  Timestamp now = Timestamp::Now();
  for (WeakConnectionList::iterator it = connectionList_.begin();
       it != connectionList_.end();) {
    TcpConnectionPtr conn = it->lock();
    if (conn) {
      Node* n = std::any_cast<Node>(conn->GetMutableContext());
      double age = TimeDifference(now, n->lastReceiveTime);
      if (age > idleSeconds_) {
        if (conn->Connected()) {
          conn->Shutdown();
          LOG_INFO << "shutting down " << conn->name();
          conn->ForceCloseWithDelay(
              3.5);  // > round trip of the whole Internet.
        }
      } else if (age < 0) {
        LOG_WARN << "Time jump";
        n->lastReceiveTime = now;
      } else {
        break;
      }
      ++it;
    } else {
      LOG_WARN << "Expired";
      it = connectionList_.erase(it);
    }
  }
}

void EchoServer::DumpConnectionList() const {
  LOG_INFO << "size = " << connectionList_.size();

  for (WeakConnectionList::const_iterator it = connectionList_.begin();
       it != connectionList_.end(); ++it) {
    TcpConnectionPtr conn = it->lock();
    if (conn) {
      printf("conn %p\n", conn.get());
      const Node& n = std::any_cast<const Node&>(conn->GetContext());
      printf("    time %s\n", n.lastReceiveTime.ToString().c_str());
    } else {
      printf("expired\n");
    }
  }
}

int main(int argc, char* argv[]) {
  EventLoop loop;
  InetAddress listenAddr(2007);
  int idle_seconds = 10;
  if (argc > 1) {
    idle_seconds = atoi(argv[1]);
  }
  LOG_INFO << "pid = " << getpid() << ", idle seconds = " << idle_seconds;
  EchoServer server(&loop, listenAddr, idle_seconds);
  server.Start();
  loop.Loop();
}

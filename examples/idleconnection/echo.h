#ifndef RYANLIB_EXAMPLES_IDLECONNECTION_ECHO_H_
#define RYANLIB_EXAMPLES_IDLECONNECTION_ECHO_H_

#include "tcp_connection/tcp_server.h"

#include <boost/circular_buffer.hpp>
#include <unordered_set>

// RFC 862
class EchoServer {
 public:
  EchoServer(EventLoop* loop,
             const InetAddress& listenAddr, int idleSeconds);

  void Start();

 private:
  void OnConnection(const TcpConnectionPtr& conn);

  void OnMessage(const TcpConnectionPtr& conn,
                 Buffer* buf, Timestamp time);

  void OnTimer();

  void DumpConnectionBuckets() const;

  using WeakTcpConnectionPtr = std::weak_ptr<TcpConnection>;

  struct Entry {
    explicit Entry(const WeakTcpConnectionPtr& weakConn)
        : weakConn_(weakConn) {}

    ~Entry() {
      TcpConnectionPtr conn = weakConn_.lock();
      if (conn) {
        conn->Shutdown();
      }
    }

    WeakTcpConnectionPtr weakConn_;
  };
  using EntryPtr = std::shared_ptr<Entry>;
  using WeakEntryPtr = std::weak_ptr<Entry>;
  using Bucket = std::unordered_set<EntryPtr>;
  using WeakConnectionList = boost::circular_buffer<Bucket>;

  TcpServer server_;
  WeakConnectionList connectionBuckets_;
};

#endif  // RYANLIB_EXAMPLES_IDLECONNECTION_ECHO_H_

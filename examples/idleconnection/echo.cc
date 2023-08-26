#include "examples/idleconnection/echo.h"

#include <cassert>
#include <cstdio>
#include <any>
#include <functional>

#include "logger/logging.h"
#include "net/event_loop.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

EchoServer::EchoServer(EventLoop* loop, const InetAddress& listenAddr,
                       int idleSeconds)
    : server_(loop, listenAddr, "EchoServer"), connectionBuckets_(idleSeconds) {
  server_.SetConnectionCallback(std::bind(&EchoServer::OnConnection, this, _1));
  server_.SetMessageCallback(
      std::bind(&EchoServer::OnMessage, this, _1, _2, _3));
  loop->RunEvery(1.0, std::bind(&EchoServer::OnTimer, this));
  connectionBuckets_.resize(idleSeconds);
  DumpConnectionBuckets();
}

void EchoServer::Start() { server_.Start(); }

void EchoServer::OnConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "EchoServer - " << conn->PeerAddress().ToIpPort() << " -> "
           << conn->LocalAddress().ToIpPort() << " is "
           << (conn->Connected() ? "UP" : "DOWN");

  if (conn->Connected()) {
    EntryPtr entry(new Entry(conn));
    connectionBuckets_.back().insert(entry);
    DumpConnectionBuckets();
    WeakEntryPtr weakEntry(entry);
    conn->SetContext(weakEntry);
  } else {
    assert(conn->GetContext().has_value());
    WeakEntryPtr weakEntry(std::any_cast<WeakEntryPtr>(conn->GetContext()));
    LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();
  }
}

void EchoServer::OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
                           Timestamp time) {
  std::string msg(buf->RetrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at "
           << time.ToString();
  conn->Send(msg);

  assert(conn->GetContext().has_value());
  WeakEntryPtr weakEntry(std::any_cast<WeakEntryPtr>(conn->GetContext()));
  EntryPtr entry(weakEntry.lock());
  if (entry) {
    connectionBuckets_.back().insert(entry);
    DumpConnectionBuckets();
  }
}

void EchoServer::OnTimer() {
  connectionBuckets_.push_back(Bucket());
  DumpConnectionBuckets();
}

void EchoServer::DumpConnectionBuckets() const {
  LOG_INFO << "size = " << connectionBuckets_.size();
  int idx = 0;
  for (WeakConnectionList::const_iterator bucketI = connectionBuckets_.begin();
       bucketI != connectionBuckets_.end(); ++bucketI, ++idx) {
    const Bucket& bucket = *bucketI;
    printf("[%d] len = %zd : ", idx, bucket.size());
    for (const auto& it : bucket) {
      bool connectionDead = it->weakConn_.expired();
      printf("%p(%ld)%s, ", it.get(), it.use_count(),
             connectionDead ? " DEAD" : "");
    }
    puts("");
  }
}

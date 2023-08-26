#include <bits/types/FILE.h>
#include <cstdio>
#include <unistd.h>
#include <memory>

#include "logger/logging.h"
#include "net/event_loop.h"
#include "tcp_connection/tcp_server.h"

void OnHighWaterMark(const TcpConnectionPtr& conn, size_t len) {
  LOG_INFO << "HighWaterMark " << len;
}

const int kBufSize = 64 * 1024;
const char* g_file = nullptr;
using FilePtr = std::shared_ptr<FILE>;

void OnConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "FileServer - " << conn->PeerAddress().ToIpPort() << " -> "
           << conn->LocalAddress().ToIpPort() << " is "
           << (conn->Connected() ? "UP" : "DOWN");
  if (conn->Connected()) {
    LOG_INFO << "FileServer - Sending file " << g_file << " to "
             << conn->PeerAddress().ToIpPort();
    conn->SetHighWatermarkCallback(OnHighWaterMark, kBufSize + 1);

    FILE* fp = ::fopen(g_file, "rb");
    if (fp) {
      FilePtr ctx(fp, ::fclose);
      conn->SetContext(ctx);
      char buf[kBufSize];
      size_t nread = ::fread(buf, 1, sizeof buf, fp);
      conn->Send(buf, static_cast<int>(nread));
    } else {
      conn->Shutdown();
      LOG_INFO << "FileServer - no such file";
    }
  }
}

void OnWriteComplete(const TcpConnectionPtr& conn) {
  const auto& fp = std::any_cast<const FilePtr&>(conn->GetContext());
  char buf[kBufSize];
  size_t nread = ::fread(buf, 1, sizeof(buf), fp.get());
  if (nread > 0) {
    conn->Send(buf, static_cast<int>(nread));
  } else {
    conn->Shutdown();
    LOG_INFO << "FileServer - done";
  }
}

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid();
  if (argc > 1) {
    g_file = argv[1];

    EventLoop loop;
    InetAddress listenAddr(2021);
    TcpServer server(&loop, listenAddr, "FileServer");
    server.SetConnectionCallback(OnConnection);
    server.SetWriteCompleteCallback(OnWriteComplete);
    server.Start();
    loop.Loop();
  } else {
    fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
  }
}

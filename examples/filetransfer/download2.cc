#include <cstdio>
#include <unistd.h>

#include "logger/logging.h"
#include "net/event_loop.h"
#include "tcp_connection/tcp_server.h"


void OnHighWaterMark(const TcpConnectionPtr& conn, size_t len) {
  LOG_INFO << "HighWaterMark " << len;
}

const int kBufSize = 64 * 1024;
const char* g_file = nullptr;

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
      conn->SetContext(fp);
      char buf[kBufSize];
      size_t nread = ::fread(buf, 1, sizeof buf, fp);
      conn->Send(buf, static_cast<int>(nread));
    } else {
      conn->Shutdown();
      LOG_INFO << "FileServer - no such file";
    }
  } else {
    if (conn->GetContext().has_value()) {
      FILE* fp = std::any_cast<FILE*>(conn->GetContext());
      if (fp) {
        ::fclose(fp);
      }
    }
  }
}

void onWriteComplete(const TcpConnectionPtr& conn) {
  FILE* fp = std::any_cast<FILE*>(conn->GetContext());
  char buf[kBufSize];
  size_t nread = ::fread(buf, 1, sizeof buf, fp);
  if (nread > 0) {
    conn->Send(buf, static_cast<int>(nread));
  } else {
    ::fclose(fp);
    fp = nullptr;
    conn->SetContext(fp);
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
    server.SetWriteCompleteCallback(onWriteComplete);
    server.Start();
    loop.Loop();
  } else {
    fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
  }
}

#include <cstdio>
#include <unistd.h>

#include "logger/logging.h"
#include "net/event_loop.h"
#include "tcp_connection/tcp_server.h"


const char* g_file = nullptr;

// FIXME: use FileUtil::ReadFile()
std::string ReadFile(const char* filename) {
  std::string content;
  FILE* fp = ::fopen(filename, "rb");
  if (fp) {
    // inefficient!!!
    const int kBufSize = 1024 * 1024;
    char iobuf[kBufSize];
    ::setbuffer(fp, iobuf, sizeof iobuf);

    char buf[kBufSize];
    size_t nread = 0;
    while ((nread = ::fread(buf, 1, sizeof buf, fp)) > 0) {
      content.append(buf, nread);
    }
    ::fclose(fp);
  }
  return content;
}

void OnHighWaterMark(const TcpConnectionPtr& conn, size_t len) {
  LOG_INFO << "HighWaterMark " << len;
}

void OnConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "FileServer - " << conn->PeerAddress().ToIpPort() << " -> "
           << conn->LocalAddress().ToIpPort() << " is "
           << (conn->Connected() ? "UP" : "DOWN");
  if (conn->Connected()) {
    LOG_INFO << "FileServer - Sending file " << g_file << " to "
             << conn->PeerAddress().ToIpPort();
    conn->SetHighWatermarkCallback(OnHighWaterMark, 64 * 1024);
    std::string fileContent = ReadFile(g_file);
    conn->Send(fileContent);
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
    server.Start();
    loop.Loop();
  } else {
    fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
  }
}

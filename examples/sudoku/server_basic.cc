#include <cstdio>
#include <unistd.h>

#include <utility>

#include "examples/sudoku/sudoku.h"
#include "logger/logging.h"
#include "base/thread.h"
#include "net/event_loop.h"
#include "net/inet_address.h"
#include "tcp_connection/tcp_server.h"


namespace {
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
} // namespace

class SudokuServer {
 public:
  SudokuServer(EventLoop* loop, const InetAddress& listenAddr)
      : server_(loop, listenAddr, "SudokuServer"),
        startTime_(Timestamp::Now()) {
    server_.SetConnectionCallback(
        std::bind(&SudokuServer::OnConnection, this, _1));
    server_.SetMessageCallback(
        std::bind(&SudokuServer::OnMessage, this, _1, _2, _3));
  }

  void Start() { server_.Start(); }

 private:
  void OnConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->PeerAddress().ToIpPort() << " -> "
              << conn->LocalAddress().ToIpPort() << " is "
              << (conn->Connected() ? "UP" : "DOWN");
  }

  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
    LOG_DEBUG << conn->name();
    size_t len = buf->ReadableBytes();
    while (len >= kCells + 2) {
      const char* crlf = buf->FindCRLF();
      if (crlf) {
        std::string request(buf->Peek(), crlf);
        buf->RetrieveUntil(crlf + 2);
        len = buf->ReadableBytes();
        if (!ProcessRequest(conn, request)) {
          conn->Send("Bad Request!\r\n");
          conn->Shutdown();
          break;
        }
      } else if (len > 100)  // id + ":" + kCells + "\r\n"
      {
        conn->Send("Id too long!\r\n");
        conn->Shutdown();
        break;
      } else {
        break;
      }
    }
  }

  bool ProcessRequest(const TcpConnectionPtr& conn, const std::string& request) {
    std::string id;
    std::string puzzle;
    bool goodRequest = true;

    std::string::const_iterator colon = find(request.begin(), request.end(), ':');
    if (colon != request.end()) {
      id.assign(request.begin(), colon);
      puzzle.assign(colon + 1, request.end());
    } else {
      puzzle = request;
    }

    if (puzzle.size() == static_cast<size_t>(kCells)) {
      LOG_DEBUG << conn->name();
      std::string result = solveSudoku(puzzle);
      if (id.empty()) {
        conn->Send(result + "\r\n");
      } else {
        conn->Send(id + ":" + result + "\r\n");
      }
    } else {
      goodRequest = false;
    }
    return goodRequest;
  }

  TcpServer server_;
  Timestamp startTime_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << ::getpid() << ", tid = " << current_thread::Tid();
  EventLoop loop;
  InetAddress listenAddr(9981);
  SudokuServer server(&loop, listenAddr);

  server.Start();

  loop.Loop();
}

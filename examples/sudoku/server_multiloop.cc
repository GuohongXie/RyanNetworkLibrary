#include <cstdio>
#include <functional>
#include <string>
#include <utility>

#include <unistd.h>

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
  SudokuServer(EventLoop* loop, const InetAddress& listen_addr, int num_event_loops)
      : server_(loop, listen_addr, "SudokuServer"),
        num_event_loops_(num_event_loops),
        start_time_(Timestamp::Now()) {
    server_.SetConnectionCallback(
        std::bind(&SudokuServer::OnConnection, this, _1));
    server_.SetMessageCallback(
        std::bind(&SudokuServer::OnMessage, this, _1, _2, _3));
    server_.SetThreadNum(num_event_loops);
  }

  void Start() {
    LOG_INFO << "starting " << num_event_loops_ << " threads.";
    server_.Start();
  }

 private:
  void OnConnection(const TcpConnectionPtr& conn) {
    LOG_TRACE << conn->PeerAddress().ToIpPort() << " -> "
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
      std::string result = SolveSudoku(puzzle);
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
  int num_event_loops_;
  Timestamp start_time_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid() << ", tid = " << current_thread::Tid();
  int num_event_loops = 0;
  if (argc > 1) {
    num_event_loops = std::stoi(argv[1]);
  }
  EventLoop loop;
  InetAddress listen_addr(9981);
  SudokuServer server(&loop, listen_addr, num_event_loops);

  server.Start();

  loop.Loop();
}

#include <cstdio>
#include <string>
#include <functional>
#include <utility>

#include <unistd.h>

#include "examples/sudoku/sudoku.h"
#include "logger/logging.h"
#include "base/thread.h"
#include "base/thread_pool.h"
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
  SudokuServer(EventLoop* loop, const InetAddress& listen_addr, int num_threads)
      : server_(loop, listen_addr, "SudokuServer"),
        num_threads_(num_threads),
        start_time_(Timestamp::Now()) {
    server_.SetConnectionCallback(
        std::bind(&SudokuServer::OnConnection, this, _1));
    server_.SetMessageCallback(
        std::bind(&SudokuServer::OnMessage, this, _1, _2, _3));
  }

  void Start() {
    LOG_INFO << "starting " << num_threads_ << " threads.";
    thread_pool_.Start(num_threads_);
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
      thread_pool_.RunTask(std::bind(&Solve, conn, puzzle, id));
    } else {
      goodRequest = false;
    }
    return goodRequest;
  }

  static void Solve(const TcpConnectionPtr& conn, const std::string& puzzle,
                    const std::string& id) {
    LOG_DEBUG << conn->name();
    std::string result = SolveSudoku(puzzle);
    if (id.empty()) {
      conn->Send(result + "\r\n");
    } else {
      conn->Send(id + ":" + result + "\r\n");
    }
  }

  TcpServer server_;
  ThreadPool thread_pool_;
  int num_threads_;
  Timestamp start_time_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid() << ", tid = " << current_thread::Tid();
  int num_threads = 0;
  if (argc > 1) {
    num_threads = std::stoi(argv[1]);
  }
  EventLoop loop;
  InetAddress listen_addr(9981);
  SudokuServer server(&loop, listen_addr, num_threads);

  server.Start();

  loop.Loop();
}

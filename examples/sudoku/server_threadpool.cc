#include <stdio.h>
#include <unistd.h>

#include <functional>
#include <utility>

#include "examples/sudoku/sudoku.h"
#include "logger/logging.h"
#include "base/thread.h"
#include "base/thread_pool.h"
#include "net/event_loop.h"
#include "net/inet_address.h"
#include "tcp_connection/tcp_server.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

class SudokuServer {
 public:
  SudokuServer(EventLoop* loop, const InetAddress& listenAddr, int num_threads)
      : server_(loop, listenAddr, "SudokuServer"),
        num_threads_(num_threads),
        start_time_(Timestamp::Now()) {
    server_.SetConnectionCallback(
        std::bind(&SudokuServer::OnConnection, this, _1));
    server_.SetMessageCallback(
        std::bind(&SudokuServer::OnMessage, this, _1, _2, _3));
  }

  void Start() {
    LOG_INFO << "starting " << num_threads_ << " threads.";
    threadPool_.Start(num_threads_);
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
      threadPool_.AddTask(std::bind(&solve, conn, puzzle, id));
    } else {
      goodRequest = false;
    }
    return goodRequest;
  }

  static void solve(const TcpConnectionPtr& conn, const std::string& puzzle,
                    const std::string& id) {
    LOG_DEBUG << conn->name();
    std::string result = solveSudoku(puzzle);
    if (id.empty()) {
      conn->Send(result + "\r\n");
    } else {
      conn->Send(id + ":" + result + "\r\n");
    }
  }

  TcpServer server_;
  ThreadPool threadPool_;
  int num_threads_;
  Timestamp start_time_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid() << ", tid = " << current_thread::Tid();
  int num_threads = 0;
  if (argc > 1) {
    num_threads = atoi(argv[1]);
  }
  EventLoop loop;
  InetAddress listenAddr(9981);
  SudokuServer server(&loop, listenAddr, num_threads);

  server.Start();

  loop.Loop();
}

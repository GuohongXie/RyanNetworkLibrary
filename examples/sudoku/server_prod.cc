#include <string>

#include <boost/circular_buffer.hpp>

#include "examples/sudoku/sudoku.h"
#include "logger/logging.h"
#include "base/thread.h"
#include "base/thread_pool.h"
#include "net/event_loop.h"
#include "net/event_loop_thread.h"
#include "net/inet_address.h"
#include "tcp_connection/tcp_server.h"

//#include <stdio.h>
//#include <unistd.h>

namespace {
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
} // namespace

class SudokuServer : Noncopyable {
 public:
  SudokuServer(EventLoop* loop, const InetAddress& listenAddr,
               int numEventLoops, int numThreads, bool nodelay)
      : server_(loop, listenAddr, "SudokuServer"),
        threadPool_(),
        numThreads_(numThreads),
        tcpNoDelay_(nodelay),
        startTime_(Timestamp::Now()) {
    LOG_INFO << "Use " << numEventLoops << " IO threads.";
    LOG_INFO << "TCP no delay " << nodelay;

    server_.SetConnectionCallback(
        std::bind(&SudokuServer::OnConnection, this, _1));
    server_.SetMessageCallback(
        std::bind(&SudokuServer::OnMessage, this, _1, _2, _3));
    server_.SetThreadNum(numEventLoops);
  }

  void Start() {
    LOG_INFO << "Starting " << numThreads_ << " computing threads.";
    threadPool_.Start(numThreads_);
    server_.Start();
  }

 private:
  void OnConnection(const TcpConnectionPtr& conn) {
    LOG_TRACE << conn->PeerAddress().ToIpPort() << " -> "
              << conn->LocalAddress().ToIpPort() << " is "
              << (conn->Connected() ? "UP" : "DOWN");
    if (conn->Connected()) {
      if (tcpNoDelay_) conn->SetTcpNoDelay(true);
      conn->SetHighWatermarkCallback(
          std::bind(&SudokuServer::HighWatermark, this, _1, _2),
          5 * 1024 * 1024);
      bool throttle = false;
      conn->SetContext(throttle);
    }
  }

  void HighWatermark(const TcpConnectionPtr& conn, size_t tosend) {
    LOG_WARN << conn->name() << " high water mark " << tosend;
    if (tosend < 10 * 1024 * 1024) {
      conn->SetHighWatermarkCallback(
          std::bind(&SudokuServer::HighWatermark, this, _1, _2),
          10 * 1024 * 1024);
      conn->SetWriteCompleteCallback(
          std::bind(&SudokuServer::WriteComplete, this, _1));
      bool throttle = true;
      conn->SetContext(throttle);
    } else {
      conn->Send("Bad Request!\r\n");
      conn->Shutdown();  // FIXME: forceClose() ?
    }
  }

  void WriteComplete(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->name() << " write complete";
    conn->SetHighWatermarkCallback(
        std::bind(&SudokuServer::HighWatermark, this, _1, _2), 5 * 1024 * 1024);
    conn->SetWriteCompleteCallback(WriteCompleteCallback());
    bool throttle = false;
    conn->SetContext(throttle);
  }

  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
                 Timestamp receiveTime) {
    size_t len = buf->ReadableBytes();
    while (len >= kCells + 2) {
      const char* crlf = buf->FindCRLF();
      if (crlf) {
        std::string request(buf->Peek(), crlf);
        buf->RetrieveUntil(crlf + 2);
        len = buf->ReadableBytes();
        if (!ProcessRequest(conn, request, receiveTime)) {
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

  struct Request {
    std::string id;
    std::string puzzle;
    Timestamp receiveTime;
  };

  bool ProcessRequest(const TcpConnectionPtr& conn, const std::string& request,
                      Timestamp receiveTime) {
    Request req;
    req.receiveTime = receiveTime;

    std::string::const_iterator colon = find(request.begin(), request.end(), ':');
    if (colon != request.end()) {
      req.id.assign(request.begin(), colon);
      req.puzzle.assign(colon + 1, request.end());
    } else {
      // when using thread pool, an id must be provided in the request.
      if (numThreads_ > 1) return false;
      req.puzzle = request;
    }

    if (req.puzzle.size() == static_cast<size_t>(kCells)) {
      bool throttle = std::any_cast<bool>(conn->GetContext());
      if (threadPool_.QueueSize() < 1000 * 1000 && !throttle) {
        threadPool_.AddTask(std::bind(&SudokuServer::Solve, this, conn, req));
      } else {
        if (req.id.empty()) {
          conn->Send("ServerTooBusy\r\n");
        } else {
          conn->Send(req.id + ":ServerTooBusy\r\n");
        }
      }
      return true;
    }
    return false;
  }

  void Solve(const TcpConnectionPtr& conn, const Request& req) {
    LOG_DEBUG << conn->name();
    std::string result = solveSudoku(req.puzzle);
    if (req.id.empty()) {
      conn->Send(result + "\r\n");
    } else {
      conn->Send(req.id + ":" + result + "\r\n");
    }
  }

  TcpServer server_;
  ThreadPool threadPool_;
  const int numThreads_;
  const bool tcpNoDelay_;
  const Timestamp startTime_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << argv[0]
           << " [number of IO threads] [number of worker threads] [-n]";
  LOG_INFO << "pid = " << getpid() << ", tid = " << current_thread::Tid();
  int numEventLoops = 0;
  int numThreads = 0;
  bool nodelay = false;
  if (argc > 1) {
    numEventLoops = std::stoi(argv[1]);
  }
  if (argc > 2) {
    numThreads = std::stoi(argv[2]);
  }
  if (argc > 3 && std::string(argv[3]) == "-n") {
    nodelay = true;
  }

  EventLoop loop;
  InetAddress listenAddr(9981);
  SudokuServer server(&loop, listenAddr, numEventLoops, numThreads, nodelay);

  server.Start();

  loop.Loop();
}

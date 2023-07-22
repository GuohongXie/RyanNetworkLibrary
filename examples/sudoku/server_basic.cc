#include <stdio.h>
#include <unistd.h>

#include <utility>

#include "sudoku.h"
#include "logging.h"
#include "thread.h"
#include "event_loop.h"
#include "inet_address.h"
#include "tcp_server.h"


class SudokuServer {
 public:
  SudokuServer(EventLoop* loop, const InetAddress& listenAddr)
      : server_(loop, listenAddr, "SudokuServer"),
        startTime_(Timestamp::Now()) {
    server_.SetConnectionCallback(
        std::bind(&SudokuServer::OnConnection, this, std::placeholders::_1));
    server_.SetMessageCallback(
        std::bind(&SudokuServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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
        if (!processRequest(conn, request)) {
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

  bool processRequest(const TcpConnectionPtr& conn, const std::string& request) {
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

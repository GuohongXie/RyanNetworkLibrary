#ifndef RYANLIB_HTTP_HTTP_SERVER_H_
#define RYANLIB_HTTP_HTTP_SERVER_H_

#include <string>

#include "logger/logging.h"
#include "base/noncopyable.h"
#include "tcp_connection/tcp_server.h"

class HttpRequest;
class HttpResponse;

class HttpServer : Noncopyable {
 public:
  using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

  HttpServer(EventLoop* loop, const InetAddress& listenAddr,
             const std::string& name,
             TcpServer::Option option = TcpServer::kNoReusePort);

  EventLoop* GetLoop() const { return server_.GetLoop(); }

  void SetHttpCallback(const HttpCallback& cb) { http_callback_ = cb; }

  void Start();

 private:
  void OnConnection(const TcpConnectionPtr& conn);
  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
                 Timestamp receiveTime);
  void OnRequest(const TcpConnectionPtr&, const HttpRequest&);

  TcpServer server_;
  HttpCallback http_callback_;
};

#endif  // RYANLIB_HTTP_HTTP_SERVER_H_
#include "http_server.h"

#include <memory>

#include "http_context.h"
#include "http_request.h"
#include "http_response.h"

/**
 * 默认的http回调函数
 * 设置响应状态码，响应信息并关闭连接
 */
void DefaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
  resp->SetStatusCode(HttpResponse::k404NotFound);
  resp->SetStatusMessage("Not Found");
  resp->SetCloseConnection(true);
}

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr,
                       const std::string& name, TcpServer::Option option)
    : server_(loop, listenAddr, name, option),
      http_callback_(DefaultHttpCallback) {
  server_.SetConnectionCallback(
      std::bind(&HttpServer::OnConnection, this, std::placeholders::_1));
  server_.SetMessageCallback(
      std::bind(&HttpServer::OnMessage, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
  server_.SetThreadNum(4);
}

void HttpServer::Start() {
  LOG_INFO << "HttpServer[" << server_.name().c_str()
           << "] starts listening on " << server_.ip_port().c_str();
  server_.Start();
}

void HttpServer::OnConnection(const TcpConnectionPtr& conn) {
  if (conn->Connected()) {
    LOG_INFO << "new Connection arrived";
  } else {
    LOG_INFO << "Connection closed";
  }
}

// 有消息到来时的业务处理
void HttpServer::OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
                           Timestamp receiveTime) {
  // LOG_INFO << "HttpServer::OnMessage";
  std::unique_ptr<HttpContext> context(new HttpContext);

#if 0
    // 打印请求报文
    std::string request = buf->GetBufferAllAsString();
    std::cout << request << std::endl;
#endif

  // 进行状态机解析
  // 错误则发送 BAD REQUEST 半关闭
  if (!context->ParseRequest(buf, receiveTime)) {
    LOG_INFO << "ParseRequest failed!";
    conn->Send("HTTP/1.1 400 Bad request\r\n\r\n");
    conn->Shutdown();
  }

  // 如果成功解析
  if (context->GotAll()) {
    LOG_INFO << "ParseRequest success!";
    OnRequest(conn, context->request());
    context->Reset();
  }
}

void HttpServer::OnRequest(const TcpConnectionPtr& conn,
                           const HttpRequest& req) {
  const std::string& connection = req.GetHeader("Connection");

  // 判断长连接还是短连接
  bool close =
      connection == "close" ||
      (req.version() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  // TODO:这里有问题，但是强制改写了
  close = true;
  // 响应信息
  HttpResponse response(close);
  // http_callback_ 由用户传入，怎么写响应体由用户决定
  // 此处初始化了一些response的信息，比如响应码，回复OK
  http_callback_(req, &response);
  Buffer buf;
  response.AppendToBuffer(&buf);
  // TODO:需要重载 TcpConnection::Send 使其可以接收一个缓冲区
  conn->Send(&buf);
  if (response.CloseConnection()) {
    conn->Shutdown();
  }
}
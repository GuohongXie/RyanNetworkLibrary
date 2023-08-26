#include <map>

#include "net/event_loop.h"
#include "tcp_connection/tcp_server.h"

using UserMap = std::map<std::string, std::string>;
UserMap users;

std::string GetUser(const std::string& user) {
  std::string result = "No such user";
  auto it = users.find(user);
  if (it != users.end()) {
    result = it->second;
  }
  return result;
}

void OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
               Timestamp receiveTime) {
  const char* crlf = buf->FindCRLF();
  if (crlf) {
    std::string user(buf->Peek(), crlf);
    conn->Send(GetUser(user) + "\r\n");
    buf->RetrieveUntil(crlf + 2);
    conn->Shutdown();
  }
}

int main() {
  users["ryan"] = "Happy and well";
  EventLoop loop;
  TcpServer server(&loop, InetAddress(1079), "Finger");
  server.SetMessageCallback(OnMessage);
  server.Start();
  loop.Loop();
}

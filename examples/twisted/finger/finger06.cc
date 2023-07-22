#include <map>

#include "event_loop.h"
#include "tcp_server.h"


typedef std::map<std::string, std::string> UserMap;
UserMap users;

std::string GetUser(const std::string& user) {
  std::string result = "No such user";
  UserMap::iterator it = users.find(user);
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
  EventLoop loop;
  TcpServer server(&loop, InetAddress(1079), "Finger");
  server.SetMessageCallback(OnMessage);
  server.Start();
  loop.Loop();
}

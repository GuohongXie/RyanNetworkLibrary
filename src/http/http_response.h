#ifndef RYANLIB_HTTP_HTTP_RESPONSE_H_
#define RYANLIB_HTTP_HTTP_RESPONSE_H_

#include <unordered_map>

class Buffer;
class HttpResponse {
 public:
  // 响应状态码
  enum HttpStatusCode {
    kUnknown,
    k200Ok = 200,
    k301MovedPermanently = 301,
    k400BadRequest = 400,
    k404NotFound = 404,
  };

  explicit HttpResponse(bool close)
      : status_code_(kUnknown), close_connection_(close) {}

  void SetStatusCode(HttpStatusCode code) { status_code_ = code; }

  void SetStatusMessage(const std::string& message) {
    status_message_ = message;
  }

  void SetCloseConnection(bool on) { close_connection_ = on; }

  bool CloseConnection() const { return close_connection_; }

  void SetContentType(const std::string& contentType) {
    AddHeader("Content-Type", contentType);
  }

  void AddHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
  }

  void SetBody(const std::string& body) { body_ = body; }

  void AppendToBuffer(Buffer* output) const;

 private:
  std::unordered_map<std::string, std::string> headers_;
  HttpStatusCode status_code_;
  // FIXME: add http version
  std::string status_message_;
  bool close_connection_;
  std::string body_;
};

#endif  // RYANLIB_HTTP_HTTP_RESPONSE_H_
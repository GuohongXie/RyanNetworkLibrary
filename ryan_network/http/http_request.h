#ifndef RYANLIB_HTTP_HTTP_REQUEST_H_
#define RYANLIB_HTTP_HTTP_REQUEST_H_

#include <map>

#include "base/timestamp.h"

class HttpRequest {
 public:
  enum Method { kInvalid, kGet, kPost, kHead, kPut, kDelete };
  enum Version { kUnknown, kHttp10, kHttp11 };

  HttpRequest() : method_(kInvalid), version_(kUnknown) {}

  void SetVersion(Version v) { version_ = v; }

  Version version() const { return version_; }

  bool SetMethod(const char* start, const char* end) {
    std::string m(start, end);
    if (m == "GET") {
      method_ = kGet;
    } else if (m == "POST") {
      method_ = kPost;
    } else if (m == "HEAD") {
      method_ = kHead;
    } else if (m == "PUT") {
      method_ = kPut;
    } else if (m == "DELETE") {
      method_ = kDelete;
    } else {
      method_ = kInvalid;
    }
    // 判断method_是否合法
    return method_ != kInvalid;
  }

  Method method() const { return method_; }

  const char* MethodString() const {
    const char* result = "UNKNOWN";
    switch (method_) {
      case kGet:
        result = "GET";
        break;
      case kPost:
        result = "POST";
        break;
      case kHead:
        result = "HEAD";
        break;
      case kPut:
        result = "PUT";
        break;
      case kDelete:
        result = "DELETE";
        break;
      default:
        break;
    }
    return result;
  }

  void SetPath(const char* start, const char* end) { path_.assign(start, end); }

  const std::string& path() const { return path_; }

  void SetQuery(const char* start, const char* end) {
    query_.assign(start, end);
  }

  const std::string& query() const { return query_; }

  void SetReceiveTime(Timestamp t) { receive_time_ = t; }

  Timestamp ReceiveTime() const { return receive_time_; }

  void AddHeader(const char* start, const char* colon, const char* end) {
    std::string field(start, colon);
    ++colon;
    // 跳过空格
    while (colon < end && ::isspace(*colon)) {
      ++colon;
    }
    std::string value(colon, end);
    // value丢掉后面的空格，通过重新截断大小设置
    while (!value.empty() && ::isspace(value[value.size() - 1])) {
      value.resize(value.size() - 1);
    }
    headers_[field] = value;
  }

  // 获取请求头部的对应值
  std::string GetHeader(const std::string& field) const {
    std::string result;
    auto it = headers_.find(field);
    if (it != headers_.end()) {
      result = it->second;
    }
    return result;
  }

  const std::map<std::string, std::string>& headers() const {
    return headers_;
  }

  void swap(HttpRequest& rhs) {
    std::swap(method_, rhs.method_);
    std::swap(version_, rhs.version_);
    path_.swap(rhs.path_);
    query_.swap(rhs.query_);
    std::swap(receive_time_, rhs.receive_time_);
    headers_.swap(rhs.headers_);
  }

 private:
  Method method_;                                         // 请求方法
  Version version_;                                       // 协议版本号
  std::string path_;                                      // 请求路径
  std::string query_;                                     // 询问参数
  Timestamp receive_time_;                                // 请求时间
  std::map<std::string, std::string> headers_;  // 请求头部列表
};

#endif  // RYANLIB_HTTP_HTTP_REQUEST_H_
#ifndef RYANLIB_BUFFER_BUFFER_H_
#define RYANLIB_BUFFER_BUFFER_H_

#include <algorithm>
#include <string>
#include <vector>

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
class Buffer {
 public:
  // prependable 初始大小，readIndex 初始位置
  static const size_t kCheapPrepend = 8;
  // writeable 初始大小，writeIndex 初始位置
  // 刚开始 readerIndex 和 writerIndex 处于同一位置
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize)
      : buffer_(kCheapPrepend + initialSize),
        reader_index_(kCheapPrepend),
        writer_index_(kCheapPrepend) {}

  /**
   * kCheapPrepend | reader | writer |
   * writer_index_ - reader_index_
   */
  size_t ReadableBytes() const { return writer_index_ - reader_index_; }
  /**
   * kCheapPrepend | reader | writer |
   * buffer_.size() - writer_index_
   */
  size_t WritableBytes() const { return buffer_.size() - writer_index_; }
  /**
   * kCheapPrepend | reader | writer |
   * wreaderIndex_
   */
  size_t PrependableBytes() const { return reader_index_; }

  // 返回缓冲区中可读数据的起始地址
  const char* Peek() const { return Begin() + reader_index_; }

  void RetrieveUntil(const char* end) { Retrieve(end - Peek()); }

  // onMessage string <- Buffer
  // 需要进行复位操作
  void Retrieve(size_t len) {
    // 应用只读取可读缓冲区数据的一部分(读取了len的长度)
    if (len < ReadableBytes()) {
      // 移动可读缓冲区指针
      reader_index_ += len;
    }
    // 全部读完 len == readableBytes()
    else {
      RetrieveAll();
    }
  }

  //void Prepend(const void* /*restrict*/ data, size_t len) {
  //  reader_index_ -= len;
  //  const char* d = static_cast<const char*>(data);
  //  std::copy(d, d + len, Begin() + reader_index_);
  //}

  // 全部读完，则直接将可读缓冲区指针移动到写缓冲区指针那
  void RetrieveAll() {
    reader_index_ = kCheapPrepend;
    writer_index_ = kCheapPrepend;
  }

  // DEBUG使用，提取出string类型，但是不会置位
  std::string GetBufferAllAsString() {
    size_t len = ReadableBytes();
    std::string result(Peek(), len);
    return result;
  }

  // 将onMessage函数上报的Buffer数据，转成string类型的数据返回
  std::string RetrieveAllAsString() {
    // 应用可读取数据的长度
    return RetrieveAsString(ReadableBytes());
  }

  std::string RetrieveAsString(size_t len) {
    // Peek()可读数据的起始地址
    std::string result(Peek(), len);
    // 上面一句把缓冲区中可读取的数据读取出来，所以要将缓冲区复位
    Retrieve(len);
    return result;
  }

  // buffer_.size() - writeIndex_
  void EnsureWritableBytes(size_t len) {
    if (WritableBytes() < len) {
      // 扩容函数
      MakeSpace(len);
    }
  }

  // string::data() 转换成字符数组，但是没有 '\0'
  void Append(const std::string& str) { Append(str.data(), str.size()); }

  // void append(const char *data)
  // {
  //     append(data, sizeof(data));
  // }

  // 把[data, data+len]内存上的数据添加到缓冲区中
  void Append(const char* data, size_t len) {
    EnsureWritableBytes(len);
    std::copy(data, data + len, BeginWrite());
    writer_index_ += len;
  }

  const char* FindCRLF() const {
    // FIXME: replace with memmem()?
    const char* crlf = std::search(Peek(), BeginWrite(), kCRLF, kCRLF + 2);
    return crlf == BeginWrite() ? NULL : crlf;
  }

  char* BeginWrite() { return Begin() + writer_index_; }

  const char* BeginWrite() const { return Begin() + writer_index_; }

  // 从fd上读取数据
  ssize_t ReadFd(int fd, int* saveErrno);
  // 通过fd发送数据
  ssize_t WriteFd(int fd, int* saveErrno);

 private:
  char* Begin() {
    // 获取buffer_起始地址
    return &(*buffer_.begin());
  }

  const char* Begin() const { return &(*buffer_.begin()); }

  // TODO:扩容操作
  void MakeSpace(int len) {
    /**
     * kCheapPrepend | reader | writer |
     * kCheapPrepend |       len         |
     */
    // 整个buffer都不够用
    if (WritableBytes() + PrependableBytes() < len + kCheapPrepend) {
      buffer_.resize(writer_index_ + len);
    } else  // 整个buffer够用，将后面移动到前面继续分配
    {
      size_t readable = ReadableBytes();
      std::copy(Begin() + reader_index_, Begin() + writer_index_,
                Begin() + kCheapPrepend);
      reader_index_ = kCheapPrepend;
      writer_index_ = reader_index_ + readable;
    }
  }

  /**
   * 采取 vector 形式，可以自动分配内存
   * 也可以提前预留空间大小、
   */
  std::vector<char> buffer_;
  size_t reader_index_;
  size_t writer_index_;
  static const char kCRLF[];
};

#endif  // RYANLIB_BUFFER_BUFFER_H_
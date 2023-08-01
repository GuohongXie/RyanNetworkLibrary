#include "logger/async_logging.h"

#include <cstdio>

#include "base/timestamp.h"

AsyncLogging::AsyncLogging(const std::string& basename, off_t roll_size,
                           int flush_interval)
    : flush_interval_(flush_interval),
      running_(false),
      basename_(basename),
      roll_size_(roll_size),
      thread_(std::bind(&AsyncLogging::ThreadFunc, this), "Logging"),
      mutex_(),
      cond_(),
      current_buffer_(new Buffer),
      next_buffer_(new Buffer),
      buffers_() {
  current_buffer_->Bzero();
  next_buffer_->Bzero();
  buffers_.reserve(16);
}

void AsyncLogging::Append(const char* logline, int len) {
  // lock在构造函数中自动绑定它的互斥体并加锁，在析构函数中解锁，大大减少了死锁的风险
  std::lock_guard<std::mutex> lock(mutex_);
  // 缓冲区剩余空间足够则直接写入
  if (current_buffer_->Avail() > len) {
    current_buffer_->Append(logline, len);
  } else {
    // 当前缓冲区空间不够，将新信息写入备用缓冲区
    buffers_.push_back(std::move(current_buffer_));
    if (next_buffer_) {
      current_buffer_ = std::move(next_buffer_);
    } else {
      // 备用缓冲区也不够时，重新分配缓冲区，这种情况很少见
      current_buffer_.reset(new Buffer);
    }
    current_buffer_->Append(logline, len);
    // 唤醒写入磁盘得后端线程
    cond_.notify_one();
  }
}

void AsyncLogging::ThreadFunc() {
  // output有写入磁盘的接口
  LogFile output(basename_, roll_size_, false);
  // 后端缓冲区，用于归还前端得缓冲区，currentBuffer nextBuffer
  BufferPtr new_buffer1(new Buffer);
  BufferPtr new_buffer2(new Buffer);
  new_buffer1->Bzero();
  new_buffer2->Bzero();
  // 缓冲区数组置为16个，用于和前端缓冲区数组进行交换
  BufferVector buffersToWrite;
  buffersToWrite.reserve(16);
  while (running_) {
    {
      // 互斥锁保护，这样别的线程在这段时间就无法向前端Buffer数组写入数据
      std::unique_lock<std::mutex> lock(mutex_);
      if (buffers_.empty()) {
        // 等待三秒也会接触阻塞
        cond_.wait_for(lock, std::chrono::seconds(3));
      }

      // 此时正使用得buffer也放入buffer数组中（没写完也放进去，避免等待太久才刷新一次）
      buffers_.push_back(std::move(current_buffer_));
      // 归还正使用缓冲区
      current_buffer_ = std::move(new_buffer1);
      // 后端缓冲区和前端缓冲区交换
      buffersToWrite.swap(buffers_);
      if (!next_buffer_) {
        next_buffer_ = std::move(new_buffer2);
      }
    }

    // 遍历所有 buffer，将其写入文件
    for (const auto& buffer : buffersToWrite) {
      output.Append(buffer->data(), buffer->Length());
    }

    // 只保留两个缓冲区
    if (buffersToWrite.size() > 2) {
      buffersToWrite.resize(2);
    }

    // 归还newBuffer1缓冲区
    if (!new_buffer1) {
      new_buffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      new_buffer1->Reset();
    }

    // 归还newBuffer2缓冲区
    if (!new_buffer2) {
      new_buffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      new_buffer2->Reset();
    }

    buffersToWrite.clear();  // 清空后端缓冲区队列
    output.Flush();          //清空文件缓冲区
  }
  output.Flush();
}
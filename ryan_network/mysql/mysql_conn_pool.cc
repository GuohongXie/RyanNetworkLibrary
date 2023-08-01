#include "mysql/mysql_conn_pool.h"

#include <cassert>
#include <fstream>
#include <thread>

MysqlConnPool* MysqlConnPool::GetConnectionPool() {
  static MysqlConnPool pool;
  return &pool;
}

MysqlConnPool::MysqlConnPool() {
  // assert();
  ParseJsonFile();

  for (int i = 0; i < min_size_; ++i) {
    AddConnection();
    current_size_++;
  }
  // 开启新线程执行任务
  std::thread producer(&MysqlConnPool::ProduceConnection, this);
  std::thread recycler(&MysqlConnPool::RecycleConnection, this);
  // 设置线程分离，不阻塞在此处
  producer.detach();
  recycler.detach();
}

MysqlConnPool::~MysqlConnPool() {
  // 释放队列里管理的MySQL连接资源
  while (!connection_queue_.empty()) {
    MysqlConn* conn = connection_queue_.front();
    connection_queue_.pop();
    delete conn;
    current_size_--;
  }
}

// 解析JSON配置文件
bool MysqlConnPool::ParseJsonFile() {
  std::ifstream file("conf.json");
  json conf = json::parse(file);

  ip_ = conf["ip"];
  user_ = conf["user_name"];
  passwd_ = conf["password"];
  db_name_ = conf["db_name"];
  port_ = conf["port"];
  min_size_ = conf["min_size"];
  max_size_ = conf["max_size"];
  timeout_ = conf["timeout"];
  max_idle_time_ = conf["max_idle_time"];
  return true;
}

void MysqlConnPool::ProduceConnection() {
  while (true) {
    // RALL手法封装的互斥锁，初始化即加锁，析构即解锁
    std::unique_lock<std::mutex> locker(mutex_);
    while (!connection_queue_.empty()) {
      // 还有可用连接则不创建
      cond_.wait(locker);
    }

    // 还没达到连接最大限制
    if (current_size_ < max_size_) {
      AddConnection();
      current_size_++;
      // 唤醒被阻塞的线程
      cond_.notify_all();
    }
  }
}

// 销毁多余的数据库连接
void MysqlConnPool::RecycleConnection() {
  while (true) {
    // 周期性的做检测工作，每500毫秒（0.5s）执行一次
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    std::lock_guard<std::mutex> locker(mutex_);
    // 连接数位于(max_size, max_size]时候，可能会有空闲连接等待太久
    while (connection_queue_.size() > min_size_) {
      MysqlConn* conn = connection_queue_.front();
      if (conn->GetAliveTime() >= max_idle_time_) {
        // 存在时间超过设定值则销毁
        connection_queue_.pop();
        delete conn;
        current_size_--;
      } else {
        // 按照先进先出顺序，前面的没有超过后面的肯定也没有
        break;
      }
    }
  }
}

void MysqlConnPool::AddConnection() {
  MysqlConn* conn = new MysqlConn;
  conn->Connect(user_, passwd_, db_name_, ip_, port_);
  conn->RefreshAliveTime();      // 刷新起始的空闲时间点
  connection_queue_.push(conn);  // 记录新连接
  current_size_++;
}

// 获取连接
std::shared_ptr<MysqlConn> MysqlConnPool::GetConnection() {
  std::unique_lock<std::mutex> locker(mutex_);
  if (connection_queue_.empty()) {
    while (connection_queue_.empty()) {
      // 如果为空，需要阻塞一段时间，等待新的可用连接
      if (std::cv_status::timeout ==
          cond_.wait_for(locker, std::chrono::milliseconds(timeout_))) {
        // std::cv_status::timeout 表示超时
        if (connection_queue_.empty()) {
          continue;
        }
      }
    }
  }

  // 有可用的连接
  // 如何还回数据库连接？
  // 使用共享智能指针并规定其删除器
  // 规定销毁后调用删除器，在互斥的情况下更新空闲时间并加入数据库连接池
  std::shared_ptr<MysqlConn> connptr(
      connection_queue_.front(), [this](MysqlConn* conn) {
        std::lock_guard<std::mutex> locker(mutex_);
        conn->RefreshAliveTime();
        connection_queue_.push(conn);
      });
  connection_queue_.pop();
  cond_.notify_all();
  return connptr;
}
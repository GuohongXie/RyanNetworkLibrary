#ifndef RYANLIB_MYSQL_MYSQL_CONN_POOL_H_
#define RYANLIB_MYSQL_MYSQL_CONN_POOL_H_

#include "mysql/mysql_conn.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#include "nlohmann_json.hpp"
using json = nlohmann::json;

class MysqlConnPool {
 public:
  static MysqlConnPool* GetConnectionPool();
  std::shared_ptr<MysqlConn> GetConnection();
  ~MysqlConnPool();

 private:
  MysqlConnPool();
  MysqlConnPool(const MysqlConnPool& obj) = delete;
  MysqlConnPool(const MysqlConnPool&& obj) = delete;
  MysqlConnPool& operator=(const MysqlConnPool& obj) = delete;

  bool ParseJsonFile();
  void ProduceConnection();
  void RecycleConnection();
  void AddConnection();

  // TODO:加上文件路径
  // std::string filePath_;
  std::string ip_;
  std::string user_;
  std::string passwd_;
  std::string db_name_;
  unsigned short port_;
  int min_size_;
  int max_size_;
  int current_size_;
  int timeout_;
  int max_idle_time_;
  std::queue<MysqlConn*> connection_queue_;  // 连接池队列
  std::mutex mutex_;
  std::condition_variable cond_;
};

#endif  // RYANLIB_MYSQL_MYSQL_CONN_POOL_H_
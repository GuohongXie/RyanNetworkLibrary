#ifndef RYANLIB_MYSQL_MYSQL_CONN_H_
#define RYANLIB_MYSQL_MYSQL_CONN_H_

#include <mysql/mysql.h>

#include <chrono>
#include <iostream>
using std::chrono::steady_clock;

class MysqlConn {
 public:
  // 初始化数据库连接
  MysqlConn();
  // 释放数据库连接
  ~MysqlConn();
  // 连接数据库
  bool Connect(const std::string& user, const std::string& passwd,
               const std::string dbName, const std::string& ip,
               const unsigned int& port = 3306);
  // 更新数据库：包括 insert Update delete 操作
  bool Update(const std::string& sql);
  // 查询数据库
  bool Query(const std::string& sql);
  // 遍历查询得到的结果集
  bool Next();
  // 得到结果集中的字段值
  std::string Value(int index);
  // 事务操作
  bool Transaction();
  // 提交事务
  bool Commit();
  // 事务回滚
  bool Rollbock();
  // 刷新起始的空闲时间点
  void RefreshAliveTime();
  // 计算连接存活的总时长
  long long GetAliveTime();

 private:
  void FreeResult();
  MYSQL* conn_ = nullptr;
  MYSQL_RES* result_ = nullptr;
  MYSQL_ROW row_ = nullptr;
  // 绝对始终
  std::chrono::steady_clock::time_point m_alivetime;
};

#endif  // RYANLIB_MYSQL_MYSQL_CONN_H_
#include <iostream>
#include <memory>
#include <thread>

#include "mysql_conn_pool.h"
#include "mysql_conn.h"
using namespace std;
// 1. 单线程: 使用/不使用连接池
// 2. 多线程: 使用/不使用连接池

// 非连接池
void Op1(int begin, int end) {
  for (int i = begin; i < end; ++i) {
    MysqlConn conn;
    conn.Connect("root", "123456", "test", "127.0.0.1");
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "insert into user values(%d, 'zhang san', '221B')", i);
    conn.Update(sql); }
}

// 连接池
void Op2(MysqlConnPool* pool, int begin, int end) {
  for (int i = begin; i < end; ++i) {
    shared_ptr<MysqlConn> conn = pool->GetConnection();
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "insert into user values(%d, 'zhang san', '221B')", i);
    conn->Update(sql);
  }
}

// 非连接池查询
void Op3(int begin, int end) {
  for (int i = begin; i < end; ++i) {
    MysqlConn conn;
    conn.Connect("root", "123456", "test", "127.0.0.1");
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select * from user");
    conn.Query(sql);
  }
}

// 连接池查询
void op4(MysqlConnPool* pool, int begin, int end) {
  for (int i = begin; i < end; ++i) {
    shared_ptr<MysqlConn> conn = pool->GetConnection();
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select * from user");
    conn->Query(sql);
  }
}

// 单线程
void Test1() {
#if 0
    // 非连接池, 单线程, 用时: 34127689958 纳秒, 34127 毫秒
    steady_clock::time_point begin = steady_clock::now();
    Op1(0, 5000);
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << "非连接池, 单线程, 用时: " << length.count() << " 纳秒, "
        << length.count() / 1000000 << " 毫秒" << endl;
#else
  // 连接池, 单线程, 用时: 19413483633 纳秒, 19413 毫秒
  MysqlConnPool* pool = MysqlConnPool::GetConnectionPool();
  steady_clock::time_point begin = steady_clock::now();
  Op2(pool, 0, 5000);
  steady_clock::time_point end = steady_clock::now();
  auto length = end - begin;
  cout << "连接池, 单线程, 用时: " << length.count() << " 纳秒, "
       << length.count() / 1000000 << " 毫秒" << endl;

#endif
}

// 多线程
void Test2() {
#if 0
    // 非连接池, 多线程, 用时: 15702495964 纳秒, 15702 毫秒
    MysqlConn conn;
    conn.Connect("root", "123456", "test", "127.0.0.1");
    steady_clock::time_point begin = steady_clock::now();
    std::thread t1(Op1, 0, 1000);
    std::thread t2(Op1, 1000, 2000);
    std::thread t3(Op1, 2000, 3000);
    std::thread t4(Op1, 3000, 4000);
    std::thread t5(Op1, 4000, 5000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << "非连接池, 多单线程, 用时: " << length.count() << " 纳秒, "
        << length.count() / 1000000 << " 毫秒" << endl;

#else
  // 连接池, 多线程, 用时: 6076443405 纳秒, 6076 毫秒
  MysqlConnPool* pool = MysqlConnPool::GetConnectionPool();
  steady_clock::time_point begin = steady_clock::now();
  std::thread t1(Op2, pool, 0, 1000);
  std::thread t2(Op2, pool, 1000, 2000);
  std::thread t3(Op2, pool, 2000, 3000);
  std::thread t4(Op2, pool, 3000, 4000);
  std::thread t5(Op2, pool, 4000, 5000);
  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  steady_clock::time_point end = steady_clock::now();
  auto length = end - begin;
  cout << "连接池, 多单线程, 用时: " << length.count() << " 纳秒, "
       << length.count() / 1000000 << " 毫秒" << endl;

#endif
}

void Test3() {
  // 连接池, 多线程, 用时: 6076443405 纳秒, 6076 毫秒
  MysqlConnPool* pool = MysqlConnPool::GetConnectionPool();
  steady_clock::time_point begin = steady_clock::now();
  std::thread t1(op4, pool, 0, 1000);
  std::thread t2(op4, pool, 1000, 2000);
  std::thread t3(op4, pool, 2000, 3000);
  std::thread t4(op4, pool, 3000, 4000);
  std::thread t5(op4, pool, 4000, 5000);
  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  steady_clock::time_point end = steady_clock::now();
  auto length = end - begin;
  cout << "连接池, 多单线程, 用时: " << length.count() << " 纳秒, "
       << length.count() / 1000000 << " 毫秒" << endl;
}

// 查询测试
int Query() {
  MysqlConn conn;
  conn.Connect("root", "123456", "test", "127.0.0.1");
  string sql = "insert into user values(1, 'zhang san', '221B')";
  bool flag = conn.Update(sql);
  cout << "flag Value:  " << flag << endl;

  sql = "select * from user";
  conn.Query(sql);
  // 从结果集中取出一行
  while (conn.Next()) {
    // 打印每行字段值
    cout << conn.Value(0) << ", " << conn.Value(1) << ", " << conn.Value(2)
         << ", " << conn.Value(3) << endl;
  }
  return 0;
}

int main() {
  // 测试MysqlConn的增删改查操作
  // Query();

  // 测试在单线程环境下，使用连接池和不使用连接池性能差距
  // Test1();

  // 测试在多线程环境下，使用连接池和不使用连接池性能差距
  Test3();

  return 0;
}
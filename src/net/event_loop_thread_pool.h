#ifndef RYANLIB_NET_EVENT_LOOP_THREAD_POOL_H_
#define RYANLIB_NET_EVENT_LOOP_THREAD_POOL_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

//此处使用前置声明
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
 public:
  // 用户传入的函数
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
  ~EventLoopThreadPool();

  // 设置线程数量
  void SetThreadNum(int numThreads) { numThreads_ = numThreads; }

  // 启动线程池
  void Start(const ThreadInitCallback& cb = ThreadInitCallback());

  // 如果工作在多线程中，baseLoop_(mainLoop)会默认以轮询的方式分配Channel给subLoop
  EventLoop* GetNextLoop();

  std::vector<EventLoop*> GetAllLoops();

  bool started() const { return started_; }
  const std::string name() const { return name_; }

 private:
  EventLoop* base_loop_;  // 用户使用muduo创建的loop 如果线程数为1
                         // 那直接使用用户创建的loop 否则创建多EventLoop
  std::string name_;
  bool started_;    // 开启线程池标志
  int num_threads_;  // 创建线程数量
  size_t next_;     // 轮询的下标
  std::vector<std::unique_ptr<EventLoopThread>> threads_;  // 保存所有的EventLoopThread容器
  std::vector<EventLoop*> loops_;  // 保存创建的所有EventLoop
};
#endif  // RYANLIB_NET_EVENT_LOOP_THREAD_POOL_H_
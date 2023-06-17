#include "event_loop_thread_pool.h"

#include <memory>

#include "event_loop_thread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,
                                         const std::string& nameArg)
    : base_loop_(baseLoop),
      name_(nameArg),
      started_(false),
      num_threads_(0),
      next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {
  // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::Start(const ThreadInitCallback& cb) {
  started_ = true;

  // 循环创建线程
  for (int i = 0; i < num_threads_; ++i) {
    char buf[name_.size() + 32];
    snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
    // 创建EventLoopThread对象
    EventLoopThread* t = new EventLoopThread(cb, buf);
    // 加入此EventLoopThread入容器
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    // 底层创建线程 绑定一个新的EventLoop 并返回该loop的地址
    // 此时已经开始执行新线程了
    loops_.push_back(t->startLoop());
  }

  // 整个服务端只有一个线程运行baseLoop
  if (num_threads_ == 0 && cb) {
    // 那么不用交给新线程去运行用户回调函数了
    cb(base_loop_);
  }
}

// 如果工作在多线程中，baseLoop_(mainLoop)会默认以轮询的方式分配Channel给subLoop
EventLoop* EventLoopThreadPool::GetNextLoop() {
  // 如果只设置一个线程 也就是只有一个mainReactor 无subReactor
  // 那么轮询只有一个线程 getNextLoop()每次都返回当前的baseLoop_
  EventLoop* loop = base_loop_;

  // 通过轮询获取下一个处理事件的loop
  // 如果没设置多线程数量，则不会进去，相当于直接返回baseLoop
  if (!loops_.empty()) {
    loop = loops_[next_];
    ++next_;
    // 轮询
    if (next_ >= loops_.size()) {
      next_ = 0;
    }
  }

  return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::GetAllLoops() {
  if (loops_.empty()) {
    return std::vector<EventLoop*>(1, base_loop_);
  } else {
    return loops_;
  }
}
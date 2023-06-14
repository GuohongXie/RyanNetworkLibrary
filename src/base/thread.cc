#include "thread.h"

#include <semaphore.h>

#include "current_thread.h"

std::atomic_int32_t Thread::num_created_(0);

Thread::Thread(ThreadFunc, const std::string& name)
    : started_(false),
      joined_(false),
      tid_(0),
      func_(std::move(func)),
      name_(name) {
  //设置线程索引编号和姓名
  SetDefaultName();
}

Thread::~Thread() {
  // thread 启动时并且未设置等待线程时才可调用
  if (started_ && !joined_) {
    //设置线程分离（守护线程，不需要等待线程结束，不会产生孤儿线程）
    thread_->detach();
  }
}

void Thread::start() {
  started_ = true;
  sem_t sem;
  ::sem_init(&sem, false, 0);
  //开启线程
  thread_ = std::shared_ptr<std::thread>(new std::thread([&]() {
    //获取线程id
    tid_ = current_thread::Tid();
    // v操作
    sem_post(&sem);
    //开启一个新线程专门执行该线程函数
    func_();
  }));
  //这里必须等待获取上面新创建的线程的tid
  //未获取到信号则不会执行sem_post,所以会被阻塞住
  //如果不使用信号量操作，则别的线程访问tid的时候，可能上面的线程还没有获取到tid
  sem_wait(&sem);
}

void Thread::join() {
  joined_ = true;
  //等待线程执行完毕
  thread_->join();
}

void Thread::SetDefaultName() {
  int num = ++num_created_;
  if (name_.empty()) {
    char buf[32] = {0};
    snprintf(buf, sizeo(buf), "Thread%d", num);
    name_ = buf;
  }
}

#ifndef RYANLIB_BASE_NONCOPYABLE_H_
#define RYANLIB_BASE_NONCOPYABLE_H_

//禁止拷贝的基类，设置为protected权限的成员函数可以让派生类继承
//派生类对象可以正常的构造和析构

// TODO: noncopyable 的学习

class Noncopyable {
 public:
  Noncopyable(const Noncopyable&) = delete;
  Noncopyable& operator=(const Noncopyable&) = delete;

 private:
  Noncopyable() = default;
  ~Noncopyable() = default;
};

#endif  // RYANLIB_BASE_NONCOPYABLE_H_

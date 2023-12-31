#ifndef RYANLIB_MEMORY_POOL_MEMORY_POOL_H_
#define RYANLIB_MEMORY_POOL_MEMORY_POOL_H_

#include <cstddef>  //size_t

//#define PAGE_SIZE 4096
//#define MP_ALIGNMENT 16
//#define mp_align(n, alignment) (((n) + (alignment - 1)) & ~(alignment - 1))
//#define mp_align_ptr(p, alignment) \
  (void*)((((size_t)p) + (alignment - 1)) & ~(alignment - 1))
constexpr size_t PAGE_SIZE = 4096;
constexpr size_t MP_ALIGNMENT = 16;

inline constexpr size_t mp_align(size_t n, size_t alignment) {
    return (n + alignment - 1) & ~(alignment - 1);
}

inline void* mp_align_ptr(void* p, size_t alignment) {
    return reinterpret_cast<void*>((reinterpret_cast<size_t>(p) + alignment - 1) & ~(alignment - 1));
}



struct SmallNode {
  unsigned char* end_;      // 该块的结尾
  unsigned char* last_;     // 该块目前使用位置
  unsigned int quote_;      // 该块被引用次数
  unsigned int failed_;     // 该块失效次数
  struct SmallNode* next_;  // 指向下一个块
};

struct LargeNode {
  void* address_;           // 该块的起始地址
  unsigned int size_;       // 该块大小
  struct LargeNode* next_;  // 指向下一个大块
};

struct Pool {
  LargeNode* large_list_;  // 管理大块内存链表
  SmallNode* head_;        // 头节点
  SmallNode*
      current_;  // 指向当前分配的块，这样可以避免遍历前面已经不能分配的块
};

class MemoryPool {
 public:
  /// @brief 默认构造，真正初始化工作交给 createPool
  MemoryPool() = default;

  /// @brief 默认析构，真正销毁工作交给 destroyPool
  ~MemoryPool() = default;

  /// @brief 初始化内存池，为 pool_ 分配 PAGE_SIZE 内存
  void CreatePool();

  /// @brief 销毁内存池，遍历大块内存和小块内存且释放它们
  void DestroyPool();

  /// @brief 申请内存的接口，内部会判断创建大块还是小块内存
  /// @param[in] size 分配内存大小
  void* malloc(size_t size);

  /// @brief 申请内存且将内存清零，内部调用 malloc
  /// @param[in] size 分配内存大小
  void* Calloc(size_t size);

  /// @brief 释放内存指定内存
  /// @param[in] p 释放内存头地址
  void FreeMemory(void* p);

  /// @brief 重置内存池
  void ResetPool();

  Pool* GetPool() { return pool_; }

 private:
  /// @brief 分配大块节点，被 malloc 调用
  /// @param[in] size 分配内存大小
  void* MallocLargeNode(size_t size);

  /// @brief 分配小块节点，被 malloc 调用
  /// @param[in] size 分配内存大小
  void* MallocSmallNode(size_t size);

  Pool* pool_ = nullptr;
};

#endif  // RYANLIB_MEMORY_POOL_MEMORY_POOL_H_
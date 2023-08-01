#include "memory_pool/memory_pool.h"

#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <vector>

using std::cout;
using std::endl;
using std::vector;

void ShowInfo(MemoryPool* Mpool, char* str) {
  printf("\r\n\r\n------start pool status %s------\r\n\r\n", str);
  Pool* pool = Mpool->GetPool();
  SmallNode* head = nullptr;
  int i = 1;
  for (head = pool->head_; head; head = head->next_, i++) {
    if (pool->current_ == head) {
      printf("current==>第%d块\n", i);
    }
    if (i == 1) {
      printf(
          "第%02d块small block  已使用:%4ld  剩余空间:%4ld  引用:%4d  "
          "failed:%4d\n",
          i, (unsigned char*)head->last_ - (unsigned char*)pool,
          head->end_ - head->last_, head->quote_, head->failed_);
    } else {
      printf(
          "第%02d块small block  已使用:%4ld  剩余空间:%4ld  引用:%4d  "
          "failed:%4d\n",
          i, (unsigned char*)head->last_ - (unsigned char*)head,
          head->end_ - head->last_, head->quote_, head->failed_);
    }
  }

  LargeNode* large;
  i = 0;
  for (large = pool->large_list_; large; large = large->next_, i++) {
    if (large->address_ != nullptr) {
      printf("第%d块large block  size=%d\n", i, large->size_);
    }
  }
  printf("\r\n\r\n------end pool status------\r\n\r\n");
}

void test1() {
  printf("sizeof(Pool) = %ld sizeof(SmallNode) = %ld\n", sizeof(Pool),
         sizeof(SmallNode));

  vector<void*> memoryVector(50);
  MemoryPool pool;
  pool.CreatePool();

  // 申请30个512字节的内存块
  for (int i = 0; i < 50; i++) {
    memoryVector[i] = pool.malloc(512);
  }
  ShowInfo(&pool, "申请50个512字节的内存块");

  // 销毁30个512字节的内存块
  for (int i = 0; i < 50; i++) {
    pool.FreeMemory(memoryVector[i]);
  }
  ShowInfo(&pool, "销毁50个512字节的内存块");

  // 申请30个5120字节的内存块
  for (int i = 0; i < 30; i++) {
    memoryVector[i] = pool.malloc(5120);
  }
  ShowInfo(&pool, "申请30个5120字节的内存块");

  // 销毁30个5120字节的内存块
  for (int i = 0; i < 30; i++) {
    pool.FreeMemory(memoryVector[i]);
  }
  ShowInfo(&pool, "销毁30个5120字节的内存块");

  pool.ResetPool();
  ShowInfo(&pool, "重置内存池");

  pool.DestroyPool();
}

#define ALLOCATE_COUNT 1000000
#define ALLOCATE_SIZE 10

void test2() {
  // vector<void*> memoryVector(100);
  void* memory[ALLOCATE_COUNT];

  cout << "use malloc free time" << endl;
  cout << "--------------------" << endl;
  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < ALLOCATE_COUNT; i++) {
    memory[i] = malloc(10);
  }
  auto end = std::chrono::steady_clock::now();
  cout << "the time cost of construct: "
       << std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count()
       << "us" << endl;

  start = std::chrono::steady_clock::now();
  for (int i = 0; i < ALLOCATE_COUNT; i++) {
    free(memory[i]);
  }
  end = std::chrono::steady_clock::now();
  cout << "the time cost of destroy: "
       << std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count()
       << "us" << endl;
  cout << "--------------------" << endl;
  cout << endl;

  MemoryPool pool;
  pool.CreatePool();

  cout << "use MemoryPool time" << endl;
  cout << "--------------------" << endl;
  start = std::chrono::steady_clock::now();
  for (int i = 0; i < ALLOCATE_COUNT; i++) {
    memory[i] = pool.malloc(10);
  }
  end = std::chrono::steady_clock::now();
  cout << "the time cost of construct: "
       << std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count()
       << "us" << endl;

  start = std::chrono::steady_clock::now();
  pool.DestroyPool();
  end = std::chrono::steady_clock::now();
  cout << "the time cost of destroy: "
       << std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count()
       << "us" << endl;
  cout << "--------------------" << endl;
  cout << endl;
}

int main() {
  // test1();

  test2();

  return 0;
}

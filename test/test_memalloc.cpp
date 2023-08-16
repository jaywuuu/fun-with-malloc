#include <gtest/gtest.h>

#include "MemoryAlloc.h"

#include <unordered_set>

TEST(MemoryAllocTest, TestNonNull) {
  MemoryAlloc::MemoryAllocator allocator(32);
  void* ptr = allocator.allocate(1);
  EXPECT_NE(ptr, nullptr);
}

static void TestAllocate(MemoryAlloc::MemoryAllocator &allocator) {
  const size_t memory_size = allocator.getMemorySize();
  std::unordered_set<void*> address_set;

  for (int i = 0; i < memory_size; ++i) {
    void* ptr = allocator.allocate(1);

    if (i < memory_size - 1) {
      EXPECT_NE(ptr, nullptr);
      EXPECT_EQ(address_set.count(ptr), 0);
    } else {
      EXPECT_EQ(ptr, nullptr);
    }
    
    address_set.insert(ptr);
  }
}

TEST(MemoryAllocTest, TestMultipleAllocations) {
  const size_t memory_size = 32;
  MemoryAlloc::MemoryAllocator allocator(memory_size);
  TestAllocate(allocator);
}

TEST(MemoryAllocTest, TestOutOfMemory) {
  MemoryAlloc::MemoryAllocator allocator(1);
  void* ptr = allocator.allocate(4);
  EXPECT_EQ(ptr, nullptr);

  allocator = MemoryAlloc::MemoryAllocator{1, 4};
  ptr = allocator.allocate(1);
  EXPECT_EQ(ptr, nullptr);
}

TEST(MemoryAllocTest, TestAllocateWithBaseAddress) {
  MemoryAlloc::MemoryAllocator allocator(2048, 4, 0xdead0000);
  void* ptr = allocator.allocate(4);
  EXPECT_GT(reinterpret_cast<size_t>(ptr), 0xdead0000);
}

TEST(MemoryAllocTest, TestAllocateAndFree) {
  const size_t memory_size = 2048;
  MemoryAlloc::MemoryAllocator allocator(memory_size, 4);
  void* ptr = allocator.allocate(64);
  EXPECT_NE(ptr, nullptr);

  allocator.free(ptr);
  void* next_ptr = allocator.allocate(64);
  EXPECT_EQ(next_ptr, ptr);
}
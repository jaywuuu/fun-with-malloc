#ifndef __memory_alloc__
#define __memory_alloc__

#include <cstddef>
#include <set>

namespace MemoryAlloc {
class MemAllocIf {
public:
  virtual void *allocate(size_t size) = 0;
  virtual void free(void *ptr) = 0;
};

class MemoryAllocator : public MemAllocIf {
public:
  MemoryAllocator(size_t available_size, size_t alignment = 1,
                  size_t base_address = 0)
      : memory_size_(available_size), alignment_(alignment),
        base_address_(base_address) {}

  void *allocate(size_t size) override;
  void free(void *ptr) override;

  size_t getMemorySize() const { return memory_size_; }

private:
  struct AddressRange {
    size_t start;
    size_t end;

    bool operator==(const AddressRange &range) const {
      return (start == range.start && end == range.end);
    }

    bool isOverlap(const AddressRange &range) const {
      return (end > range.start && start < range.end);
    }
  };

  struct AddressRangeCompare {
    bool operator()(const AddressRange &lhs, const AddressRange &rhs) const {
      return (lhs.start < rhs.start);
    }
  };

  size_t memory_size_ = 0;
  size_t bytes_used_ = 0;
  size_t alignment_ = 1;
  size_t base_address_ = 0;

  std::set<AddressRange, AddressRangeCompare> used_address_;

protected:
  size_t getAddress(size_t size);
  bool checkRange(AddressRange &range);
};

class SimpleSeqFitAlloc : public MemAllocIf {
public:
  struct BlockHeader {
    BlockHeader *next = nullptr;
    size_t size = 0;
  };

  SimpleSeqFitAlloc(uint8_t *base_address, size_t size,
                    size_t alignment = sizeof(size_t));

  void *allocate(size_t size) override;
  void free(void *ptr) override;

private:
  void *allocate_private(size_t size);
  void coalesce();

  uint8_t *base_address_ = nullptr;
  BlockHeader *free_header_ = nullptr;
  size_t size_ = 0;
  size_t alignment_ = 0;
};

} // namespace MemoryAlloc

#endif /* __memory_alloc__ */
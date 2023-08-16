
#include "MemoryAlloc.h"

#include <cassert>
#include <cstdint>

namespace MemoryAlloc {

void *MemoryAllocator::allocate(size_t size) {
  size_t algned_size = (size + alignment_ - 1) & ~(alignment_ - 1);
  size_t used = algned_size + bytes_used_;

  // overflow
  if (used < size || used < bytes_used_)
    return nullptr;

  // Not enough memory
  if (used > memory_size_)
    return nullptr;

  // Get address.
  size_t address = getAddress(algned_size);

  if (address)
    bytes_used_ += algned_size;

  return reinterpret_cast<void *>(address);
}

void MemoryAllocator::free(void *ptr) {
  size_t address = reinterpret_cast<size_t>(ptr);
  MemoryAllocator::AddressRange range = {address, address};
  auto result = used_address_.find(range);

  if (result == used_address_.end()) {
    // error?
    return;
  }

  size_t size = result->end - result->start;
  bytes_used_ -= size;

  used_address_.erase(result);
}

size_t MemoryAllocator::getAddress(size_t size) {
  size_t address = alignment_;
  MemoryAllocator::AddressRange range = {};

  for (; address + size <= memory_size_; address += alignment_) {
    MemoryAllocator::AddressRange candidate = {address, address + size};
    if (!checkRange(candidate))
      continue;

    range = candidate;
    break;
  }

  if (range.start == 0)
    return 0;

  used_address_.insert(range);

  address += base_address_;
  return address;
}

bool MemoryAllocator::checkRange(MemoryAllocator::AddressRange &range) {
  if (used_address_.count(range) != 0)
    return false;

  // TODO: binary search
  for (auto &r : used_address_) {
    if (range.isOverlap(r)) {
      return false;
    }
  }

  return true;
}

SimpleSeqFitAlloc::SimpleSeqFitAlloc(uint8_t *base_address, size_t size,
                                     size_t alignment)
    : base_address_(base_address), size_(size), alignment_(alignment) {
  assert(base_address != nullptr);

  SimpleSeqFitAlloc::BlockHeader *header =
      reinterpret_cast<SimpleSeqFitAlloc::BlockHeader *>(base_address);
  header->next = nullptr;
  header->size = size;

  free_header_ = header;
}

void SimpleSeqFitAlloc::coalesce() {
  SimpleSeqFitAlloc::BlockHeader *header = free_header_;
  while (header != nullptr) {
    // We can merge current free block with the next one if:
    // current ptr + block size + header size == next ptr.
    uint8_t *next_addr = reinterpret_cast<uint8_t *>(header) + header->size +
                         sizeof(SimpleSeqFitAlloc::BlockHeader);
    if (next_addr == reinterpret_cast<uint8_t *>(header->next)) {
      // merge blocks.
      SimpleSeqFitAlloc::BlockHeader *next = header->next;
      header->size = header->size + next->size;
      header->next = next->next;
      continue;
    }

    header = header->next;
  }
}

void *SimpleSeqFitAlloc::allocate_private(size_t size) {
  SimpleSeqFitAlloc::BlockHeader *header = free_header_;
  while (header != nullptr &&
         size >= header->size - sizeof(SimpleSeqFitAlloc::BlockHeader)) {
    header = header->next;
  }

  if (header == nullptr) {
    return nullptr;
  }

  if (size >= header->size - sizeof(SimpleSeqFitAlloc::BlockHeader)) {
    return nullptr;
  }

  // Create new block header at the end of size.
  uint8_t *free_addr = reinterpret_cast<uint8_t *>(header) + size +
                       sizeof(SimpleSeqFitAlloc::BlockHeader);
  SimpleSeqFitAlloc::BlockHeader *new_header =
      reinterpret_cast<SimpleSeqFitAlloc::BlockHeader *>(free_addr);
  new_header->size =
      header->size - size - sizeof(SimpleSeqFitAlloc::BlockHeader);
  new_header->next = header->next;

  header->size = size;
  header->next = nullptr;

  free_header_ = new_header;

  return nullptr;
}

void *SimpleSeqFitAlloc::allocate(size_t size) {
  size_t algned_size = (size + alignment_ - 1) & ~(alignment_ - 1);
  void *ptr = allocate_private(algned_size);
  if (ptr == nullptr) {
    // coalesce and try again.
    coalesce();
    ptr = allocate_private(algned_size);
  }

  return ptr;
}

void SimpleSeqFitAlloc::free(void *ptr) {
  // Get header of address.
  uint8_t *addr =
      reinterpret_cast<uint8_t *>(ptr) - sizeof(SimpleSeqFitAlloc::BlockHeader);
  SimpleSeqFitAlloc::BlockHeader *freed_header =
      reinterpret_cast<SimpleSeqFitAlloc::BlockHeader *>(addr);

  // If the freed block is before the current free header pointer, then we can
  // just update the free header pointer.
  if (freed_header < free_header_) {
    free_header_ = freed_header;
    freed_header->next = free_header_;
    return;
  }

  // Scan from free_header_ and look for correct place to insert freed block.
  SimpleSeqFitAlloc::BlockHeader *header = free_header_;
  SimpleSeqFitAlloc::BlockHeader *prev = nullptr;
  while (header != nullptr && header < freed_header) {
    prev = header;
    header = header->next;
  }
  prev->next = freed_header;
  freed_header->next = header;

  // See if we should coalesce.
  addr = reinterpret_cast<uint8_t *>(prev) +
         sizeof(SimpleSeqFitAlloc::BlockHeader) + prev->size;
  if (addr == reinterpret_cast<uint8_t *>(freed_header)) {
    coalesce();
  }
}

} // namespace MemoryAlloc
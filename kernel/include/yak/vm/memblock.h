#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <yak/vm/address.h>
#include <yak/vm/alloc.h>

namespace yak {
constexpr int MEMBLOCK_MAX_REGIONS = 128;

struct MemblockRegion {
  paddr_t base;
  size_t size;
  int node_id;

  paddr_t end() const { return base + size; }
};

struct MemblockType {
  friend struct Memblock;

  size_t total_size = 0;
  int count = 0;
  MemblockRegion regions[MEMBLOCK_MAX_REGIONS];

  std::span<MemblockRegion> view() {
    return {regions, static_cast<size_t>(count)};
  }

  std::span<const MemblockRegion> view() const {
    return {regions, static_cast<size_t>(count)};
  }

  bool is_full() const { return count >= MEMBLOCK_MAX_REGIONS; }

  void add(paddr_t base, size_t size, int nid);

  void print() const;

  void coalesce();

private:
  int find_insert_index(paddr_t base) const;
  void insert(int index, paddr_t base, size_t size, int nid);
  void remove(int index);
};

struct Memblock {
  MemblockType usable;
  MemblockType reserved;

  inline void coalesce_blocks() {
    usable.coalesce();
    reserved.coalesce();
  }

  std::optional<paddr_t> allocate(size_t size, size_t align, int nid);
  std::optional<vaddr_t> allocate_virtual(size_t size, size_t align, int nid);

  void assign_node_to_range(paddr_t base, size_t size, int nid);

private:
  std::optional<paddr_t> allocate_from_region(const MemblockRegion &region,
                                              size_t size, size_t align) const;
  std::optional<paddr_t> try_allocate(size_t size, size_t align, int nid);
};

extern Memblock boot_memblock;

} // namespace yak

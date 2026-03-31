#include <algorithm>
#include <span>
#include <yak/arch-mm.h>
#include <yak/log.h>
#include <yak/math.h>
#include <yak/vm/alloc.h>
#include <yak/vm/memblock.h>

namespace yak {

Memblock boot_memblock = Memblock();

void MemblockType::add(paddr_t base, size_t size, int nid) {
  if (is_full()) {
    pr_error("too many regions; can't add more\n");
    return;
  }
  insert(find_insert_index(base), base, size, nid);
}

int MemblockType::find_insert_index(paddr_t base) const {
  auto v = view();
  auto it = std::lower_bound(
      v.begin(), v.end(), base,
      [](const MemblockRegion &r, paddr_t b) { return r.base < b; });
  return static_cast<int>(std::distance(v.begin(), it));
}

void MemblockType::insert(int index, paddr_t base, size_t size, int nid) {
  auto v = std::span(regions, static_cast<size_t>(count + 1));
  std::shift_right(v.begin() + index, v.end(), 1);

  regions[index] = {.base = base, .size = size, .node_id = nid};
  total_size += size;
  count++;
}

void MemblockType::remove(int index) {
  auto v = view();
  total_size -= regions[index].size;
  std::shift_left(v.begin() + index, v.end(), 1);
  count--;
}

void MemblockType::coalesce() {
  for (int i = 0; i < count - 1;) {
    auto &current = regions[i];
    auto &next = regions[i + 1];

    if (current.node_id == next.node_id && current.end() == next.base) {
      current.size += next.size;
      total_size += next.size;
      remove(i + 1);
    } else {
      i++;
    }
  }
}

void MemblockType::print() const {
  pr_debug("MemblockType: total_size=%zu count=%d\n", total_size, count);

  for (int i = 0; i < count; ++i) {
    const auto &r = regions[i];

    pr_debug("  [%d] base=%#llx end=%#llx size=%#zx nid=%d\n", i,
             static_cast<unsigned long long>(r.base),
             static_cast<unsigned long long>(r.end()), r.size, r.node_id);
  }
}

std::optional<paddr_t> Memblock::try_allocate(size_t size, size_t align,
                                              int nid) {
  // Iterate memory regions from highest to lowest
  auto regions = usable.view();
  for (auto it = regions.rbegin(); it != regions.rend(); ++it) {
    if (nid != NUMA_ANY && it->node_id != nid)
      continue;
    if (it->size < size)
      continue;

    paddr_t candidate = align_down(it->end() - size, align);
    if (candidate < it->base)
      continue;

    const int region_nid = it->node_id;
    const paddr_t region_base = it->base;
    const paddr_t region_end = it->end();
    const int index =
        static_cast<int>(std::distance(regions.begin(), it.base()) - 1);

    usable.remove(index);

    if (candidate > region_base)
      usable.add(region_base, candidate - region_base, region_nid);

    if (candidate + size < region_end)
      usable.add(candidate + size, region_end - (candidate + size), region_nid);

    reserved.add(candidate, size, region_nid);

    return candidate;
  }

  return std::nullopt;
}

std::optional<paddr_t> Memblock::allocate(size_t size, size_t align, int nid) {
  if (nid == NUMA_LOCAL) {
    pr_warn("implement numa local memory id\n");
    nid = 0;
  }

  if (auto addr = try_allocate(size, align, nid))
    return addr;

  return std::nullopt;
}

std::optional<vaddr_t> Memblock::allocate_virtual(size_t size, size_t align,
                                                  int nid) {
  return allocate(size, align, nid).transform(arch::p2v);
}

void Memblock::assign_node_to_range(paddr_t base, size_t size, int nid) {
  const paddr_t range_end = base + size;

  for (int i = 0; i < usable.count;) {
    auto &r = usable.regions[i];

    const paddr_t overlap_start = std::max(r.base, base);
    const paddr_t overlap_end = std::min(r.end(), range_end);

    if (overlap_start >= overlap_end) {
      i++;
      continue;
    }

    const paddr_t r_base = r.base;
    const paddr_t r_end = r.end();
    const int r_nid = r.node_id;

    usable.remove(i);

    if (r_base < overlap_start)
      usable.add(r_base, overlap_start - r_base, r_nid);

    usable.add(overlap_start, overlap_end - overlap_start, nid);

    if (overlap_end < r_end)
      usable.add(overlap_end, r_end - overlap_end, r_nid);

    i++;
  }
}

} // namespace yak

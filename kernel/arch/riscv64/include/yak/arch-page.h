#pragma once

#include <yak/arch-mm.h>
#include <yak/vm/page.h>

namespace yak::arch {

[[gnu::const]]
inline Page &p2page(paddr_t pa) {
  auto pfn = p2pfn(pa);
  auto pfndb = reinterpret_cast<Page *>(PFNDB_BASE);
  return pfndb[pfn];
}

} // namespace yak::arch

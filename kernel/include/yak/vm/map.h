#pragma once

#include <yak/arch-pagemap.h>

namespace yak {
class VmMap {
public:
  void bootstrap_kernel();
  void activate();

  inline PageMap &page_map() {
    return pm;
  }

private:
  PageMap pm;
};

extern VmMap kmap;

} // namespace yak

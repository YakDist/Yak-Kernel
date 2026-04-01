#include <yak/vm/map.h>

namespace yak {
VmMap kmap = VmMap();

void VmMap::bootstrap_kernel() {
  pm.bootstrap_kernel();
}

void VmMap::activate() {
  pm.activate();
}
} // namespace yak

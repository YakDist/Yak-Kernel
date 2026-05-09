#include <atomic>
#include <yak/ipl.h>

namespace yak {

static std::atomic<Ipl> g_ipl;

Ipl iplget() {
  return g_ipl.load(std::memory_order_relaxed);
}

Ipl iplr(Ipl ipl) {
  return g_ipl.exchange(ipl, std::memory_order_relaxed);
}

void iplx(Ipl ipl) {
  g_ipl.store(ipl, std::memory_order_relaxed);
}

} // namespace yak

#include <yak/cpudata.h>
#include <yak/ipl.h>
#include <yak/panic.h>
#include <yak/percpu.h>
#include <yak/softint.h>

namespace yak {

Ipl iplget() {
  return CPUDATA_LOAD(md.soft_ipl);
}

Ipl iplr(Ipl ipl) {
  if (ipl < iplget()) [[unlikely]] {
    panic("IPL: raise ipl < current\n");
  }
  return CPUDATA_XCHG(md.soft_ipl, ipl);
}

void iplx(Ipl ipl) {
  CPUDATA_STORE(md.soft_ipl, ipl);
  if (CPUDATA_LOAD(md.hard_ipl) > ipl) {
    CPUDATA_STORE(md.hard_ipl, ipl);
    // TODO: lower hard ipl :-(
  }

  if (CpuData::Current()->softints_pending.load(std::memory_order_relaxed) >>
      std::to_underlying(ipl)) {
    softint_dispatch(ipl);
  }
}

} // namespace yak

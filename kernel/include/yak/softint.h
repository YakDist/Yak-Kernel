#pragma once

#include <yak/cpudata.h>
#include <yak/ipl.h>

namespace yak {
void softint_dispatch(Ipl ipl);
void softint_issue(Ipl ipl);
void softint_issue_other(CpuData *cpu, Ipl ipl);
} // namespace yak

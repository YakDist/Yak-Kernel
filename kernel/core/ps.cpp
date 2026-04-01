#include <cstddef>
#include <frg/mutex.hpp>
#include <yak/bitset.h>
#include <yak/ps.h>

namespace yak {

static AtomicBitset<PID_MAX_LIMIT> pids_allocated{};
// acts as allocation "cache"
static std::atomic<size_t> last_pid{0};

pid_t allocate_pid() {
  size_t start = last_pid.load(std::memory_order_relaxed);

  for (size_t i = 0; i < PID_MAX_LIMIT; i++) {
    size_t pid = (start + i) % PID_MAX_LIMIT;

    if (pids_allocated.compare_exchange_bit(pid, false, true)) {
      size_t expected = start;
      last_pid.compare_exchange_strong(expected, pid + 1,
                                       std::memory_order_relaxed);
      return static_cast<pid_t>(pid);
    }
  }

  return -1;
}

Process kernel_process = Process();

Process::Process(Process *parent)
    : state{ProcessState::PROCESS_ALIVE},
      childlist{} {

  pid = allocate_pid();

  if (parent != nullptr) {
    auto guard = frg::guard(&parent->childlist_lock);
    parent->childlist.push_back(this);
  }
}

} // namespace yak

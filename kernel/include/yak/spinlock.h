#pragma once

#include <yak/assert.h>
#include <atomic>
#include <yak/arch-intr.h>
#include <yak/ipl.h>

namespace yak {

#ifdef x86_64
#define busyloop_hint() asm volatile("pause" ::: "memory")
#elifdef riscv64
// This is 'pause' (encoded as fence w, 0)
// On Zihintpause chips, it saves power
// On chips not supporting this instr. it's equivalent to a nop,
//  thus the 4byte encoding
#define busyloop_hint() asm volatile(".4byte 0x0100000f" ::: "memory")
#else
#warning "unimplemented busyloop_hint"
#define busyloop_hint()
#endif

class Spinlock {
public:
  constexpr Spinlock() : locked{false} {}
  ~Spinlock() = default;

  Spinlock(const Spinlock &) = delete;
  Spinlock &operator=(const Spinlock &) = delete;
  Spinlock(Spinlock &&other) noexcept = delete;
  Spinlock &operator=(Spinlock &&other) noexcept = delete;

  bool is_locked() {
    return locked.load(std::memory_order_acquire);
  }

  bool try_lock() {
    bool expected = false;
    return locked.compare_exchange_weak(
        expected, true, std::memory_order_acq_rel, std::memory_order_relaxed);
  }

  void lock() {
    // Spinlocks may only be acquired with interrupts disabled or at
    // IPL >= dispatch. This is enforced in debug builds.
    assert(!arch::interrupt_state() || iplget() >= Ipl::dispatch);
    while (!try_lock()) {
      busyloop_hint();
    }
  }

  void unlock() {
    locked.store(false, std::memory_order_release);
  }

private:
  std::atomic<bool> locked;
};

} // namespace yak

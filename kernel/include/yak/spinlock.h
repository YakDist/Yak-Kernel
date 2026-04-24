#pragma once

#include <atomic>
#include <yak/arch-intr.h>
#include <yak/ipl.h>

namespace yak {

#ifdef x86_64
#define busyloop_hint() asm volatile("pause" ::: "memory");
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

class IplSpinlock : Spinlock {
public:
  constexpr IplSpinlock() : Spinlock(), prev_ipl(Ipl::passive) {}

  IplSpinlock(const IplSpinlock &) = delete;
  IplSpinlock &operator=(const IplSpinlock &) = delete;
  IplSpinlock(IplSpinlock &&other) noexcept = delete;
  IplSpinlock &operator=(IplSpinlock &&other) noexcept = delete;

  bool is_locked() {
    return Spinlock::is_locked();
  }

  bool try_lock(Ipl at = Ipl::dispatch) {
    Ipl old = iplr(at);
    bool result = Spinlock::try_lock();
    if (!result) {
      iplx(old);
    } else {
      prev_ipl = old;
    }
    return result;
  }

  void lock(Ipl at = Ipl::dispatch) {
    Ipl old = iplr(at);
    Spinlock::lock();
    this->prev_ipl = old;
  }

  void unlock() {
    Spinlock::unlock();
    iplx(prev_ipl);
  }

  void lock_noipl() {
    Spinlock::lock();
  }

  void unlock_noipl() {
    Spinlock::unlock();
  }

private:
  Ipl prev_ipl;
};

class InterruptSpinLock : Spinlock {
public:
  constexpr InterruptSpinLock() : Spinlock(), prev_interrupts(false) {}
  InterruptSpinLock(const InterruptSpinLock &) = delete;
  InterruptSpinLock &operator=(const InterruptSpinLock &) = delete;
  InterruptSpinLock(InterruptSpinLock &&) noexcept = delete;
  InterruptSpinLock &operator=(InterruptSpinLock &&) noexcept = delete;

  bool try_lock() {
    bool old = arch::interrupt_state();
    arch::disable_interrupts();
    bool result = Spinlock::try_lock();
    if (!result) {
      if (old)
        arch::enable_interrupts();
    } else {
      prev_interrupts = old;
    }
    return result;
  }

  void lock() {
    bool old = arch::interrupt_state();
    arch::disable_interrupts();
    Spinlock::lock();
    prev_interrupts = old;
  }

  void unlock() {
    Spinlock::unlock();
    if (prev_interrupts)
      arch::enable_interrupts();
  }

private:
  bool prev_interrupts;
};

} // namespace yak

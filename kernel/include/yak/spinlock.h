#pragma once

#include <atomic>
#include <yak/arch-intr.h>
#include <yak/ipl.h>

namespace yak {

#ifdef x86_64
#define busyloop_hint() asm volatile("pause" ::: "memory");
#else
#warning "unimplemented busyloop_hint"
#define busyloop_hint()
#endif

class SpinLock {
public:
  constexpr SpinLock() : locked{false} {}
  ~SpinLock() = default;

  SpinLock(const SpinLock &) = delete;
  SpinLock &operator=(const SpinLock &) = delete;
  SpinLock(SpinLock &&other) noexcept = delete;
  SpinLock &operator=(SpinLock &&other) noexcept = delete;

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

  void unlock() { locked.store(false, std::memory_order_release); }

private:
  std::atomic<bool> locked;
};

class IplSpinLock : SpinLock {
public:
  constexpr IplSpinLock() : SpinLock(), prev_ipl(Ipl::passive) {}

  IplSpinLock(const IplSpinLock &) = delete;
  IplSpinLock &operator=(const IplSpinLock &) = delete;
  IplSpinLock(IplSpinLock &&other) noexcept = delete;
  IplSpinLock &operator=(IplSpinLock &&other) noexcept = delete;

  bool try_lock(Ipl at = Ipl::dispatch) {
    Ipl old = iplr(at);
    bool result = SpinLock::try_lock();
    if (!result) {
      iplx(old);
    } else {
      prev_ipl = old;
    }
    return result;
  }

  void lock(Ipl at = Ipl::dispatch) {
    Ipl old = iplr(at);
    SpinLock::lock();
    this->prev_ipl = old;
  }

  void unlock() {
    SpinLock::unlock();
    iplx(prev_ipl);
  }

private:
  Ipl prev_ipl;
};

class InterruptSpinLock : SpinLock {
public:
  constexpr InterruptSpinLock() : SpinLock(), prev_interrupts(false) {}
  InterruptSpinLock(const InterruptSpinLock &) = delete;
  InterruptSpinLock &operator=(const InterruptSpinLock &) = delete;
  InterruptSpinLock(InterruptSpinLock &&) noexcept = delete;
  InterruptSpinLock &operator=(InterruptSpinLock &&) noexcept = delete;

  bool try_lock() {
    bool old = arch::interrupt_state();
    arch::disable_interrupts();
    bool result = SpinLock::try_lock();
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
    SpinLock::lock();
    prev_interrupts = old;
  }

  void unlock() {
    SpinLock::unlock();
    if (prev_interrupts)
      arch::enable_interrupts();
  }

private:
  bool prev_interrupts;
};

} // namespace yak

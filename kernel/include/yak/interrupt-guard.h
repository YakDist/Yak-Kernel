#pragma once

#include <yak/assert.h>
#include <yak/arch-intr.h>

namespace yak {
class InterruptGuard {
public:
  explicit InterruptGuard()
      : saved_(arch::interrupt_state()),
        dismissed_(false) {
    arch::disable_interrupts();
  }

  ~InterruptGuard() {
    if (!dismissed_ && saved_)
      arch::enable_interrupts();
  }

  void unlock() {
    assert(!dismissed_);
    dismissed_ = true;
    if (saved_)
      arch::enable_interrupts();
  }

  InterruptGuard(const InterruptGuard &) = delete;
  InterruptGuard &operator=(const InterruptGuard &) = delete;
  InterruptGuard(InterruptGuard &&) = delete;
  InterruptGuard &operator=(InterruptGuard &&) = delete;

private:
  bool saved_;
  bool dismissed_;
};
} // namespace yak

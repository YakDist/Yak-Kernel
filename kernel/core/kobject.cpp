#include <yak/assert.h>
#include <frg/mutex.hpp>
#include <yak/kobject.h>
#include <yak/panic.h>
#include <yak/thread.h>
#include <yak/waitblock.h>

namespace yak {
int KObject::signal_locked(bool unblock_all) {
  assert(lock_.is_locked());
  assert(iplget() == Ipl::dispatch);

  int unblocked = 0;

  while (wait_count_) {
    auto *wb = wait_list_.front();
    assert(wb);

    auto *thread = wb->thread_;

    auto guard = frg::guard(&thread->lock_);

    wb->flags_ |= WB_DEQUEUED;
    wait_list_.pop_front();
    wait_count_ -= 1;

    if (wb->flags_ & WB_UNWAITED) {
      // wb was already unwaited somewhere else
      continue;
    }

    thread->unwait(wb->wake_status_);

    if (!unblock_all) {
      return 1;
    }

    ++unblocked;
  }

  return unblocked;
}
} // namespace yak

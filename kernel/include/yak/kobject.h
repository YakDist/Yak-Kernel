#pragma once

#include <cstddef>
#include <frg/list.hpp>
#include <yak/spinlock.h>
#include <yak/waitblock.h>

namespace yak {
enum class KObjectType {
  // A Notify object will not have
  // it's signalstate decremented on wait
  Notify,
  // While a Sync object will have it's
  // signalstate decremented on wait!
  //
  // Examples include:
  // semaphores, (sync) events, ...
  Sync,
};

struct KObject {
  KObject(int signalstate, KObjectType type)
      : lock_{},
        type_{type},
        signal_count_{signalstate},
        wait_count_{0},
        wait_list_{} {}

  KObject(const KObject &) = delete;
  KObject &operator=(const KObject &) = delete;
  KObject(KObject &&) = delete;
  KObject &operator=(KObject &&) = delete;

  inline bool is_signaled() const {
    return signal_count_ > 0;
  }

  // Caller must hold obj_lock
  // returns number of threads woken
  int signal_locked(bool unblock_all);

  Spinlock lock_;
  KObjectType type_;
  // <=0 unsignaled, >0 signaled
  long signal_count_;
  // number of threads waiting on object
  size_t wait_count_;
  WaitBlockList wait_list_;
};

} // namespace yak

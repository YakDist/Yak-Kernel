#pragma once

#include <frg/list.hpp>
#include <yak/spinlock.h>

namespace yak {

struct Dpc;
using DpcCallback = void (*)(Dpc *, void *);

class DpcQueue;
struct Dpc {
  DpcQueue *enqueued_to_;

  DpcCallback callback_;
  void *context_;

  frg::default_list_hook<Dpc> hook;

  void init(DpcCallback callback);
  void dequeue();
};

using DpcList = frg::intrusive_list<
    Dpc, frg::locate_member<Dpc, frg::default_list_hook<Dpc>, &Dpc::hook>>;

class DpcQueue {
  friend struct Dpc;

public:
  void drain();
  static void enqueue(Dpc *dpc, void *context);

private:
  // This needs to be an InterruptSpinLock, as DPCs have to be enqueueable from
  // interrupt context
  InterruptSpinLock queue_lock;
  DpcList queue;
};
} // namespace yak

#pragma once

#include <frg/list.hpp>
#include <frg/string.hpp>
#include <yak/arch-pcb.h>
#include <yak/sched_prio.h>
#include <yak/spinlock.h>

namespace yak {
enum class ThreadState {
  // freshly created
  Undefined,
  // enqueued on runqueue
  Queued,
  // off-list, set as next thread
  WaitingForSwitch,
  // off-list, currently switching to this thread
  Switching,
  // off-list, currently active
  Running,
  // off-list, block until wakeup
  Blocked,
  // off-list, terminating
  Terminating,
  // enqueued on reaper queue
  Dead
};

// This implementation is derived from microsoft's channel9:
// "Inside Windows 7: Arun Kishan - Farewell to the Windows Kernel
// Dispatcher Lock"
enum class WaitPhase {
  None,
  // Thread currently setting up wait machinery
  InProgress,
  // Thread comitted to waiting
  Comitted,
  // The wait was aborted whilst InProgress
  Aborted,
};

class Process;
struct CpuData;

static constexpr size_t THREAD_MAX_NAME_LEN = 32;

using ThreadEntryFn = void (*)(void *, void *);

struct Thread {
  arch::ThreadPcb md;

  IplSpinlock lock;

  ThreadState state;
  SchedPrio priority;

  Process *parent_process;
  Process *effective_process;

  CpuData *affinity_cpu = nullptr;
  CpuData *last_cpu = nullptr;

  void *kernel_stack_top = nullptr;

  std::atomic<bool> is_switching;

  bool is_user;

  char name[THREAD_MAX_NAME_LEN];

  frg::default_list_hook<Thread> list_hook;
  frg::default_list_hook<Thread> queue_hook;

  Thread(frg::string_view name, SchedPrio initial_priority,
         Process *parent_process, bool user_thread);

  static Thread *Current();

  void init_context(void *kstack_top, ThreadEntryFn entry, void *ctx1,
                    void *ctx2);

  [[noreturn]]
  void exit();
};

using ThreadList = frg::intrusive_list<
    Thread, frg::locate_member<Thread, frg::default_list_hook<Thread>,
                               &Thread::list_hook>>;

using ThreadQueue = frg::intrusive_list<
    Thread, frg::locate_member<Thread, frg::default_list_hook<Thread>,
                               &Thread::queue_hook>>;

} // namespace yak

#include <algorithm>
#include <string.h>
#include <yak/cpudata.h>
#include <yak/ipl.h>
#include <yak/log.h>
#include <yak/panic.h>
#include <yak/sched.h>
#include <yak/thread.h>

namespace yak {
Thread::Thread(frg::string_view name, SchedPrio initial_priority,
               Process *parent_process, bool user_thread)
    : state{ThreadState::Undefined},
      priority{initial_priority},
      parent_process{parent_process},
      effective_process{parent_process},
      is_user{user_thread} {
  auto name_copy_len = std::min(name.size() - 1, THREAD_MAX_NAME_LEN - 1);
  memcpy(this->name, name.data(), name_copy_len);
}

Thread *Thread::Current() {
  return CPUDATA_LOAD(current_thread);
}

[[noreturn]]
void Thread::exit() {
  iplr(Ipl::dispatch);

  // XXX: switch to kernel map?
  if (is_user)
    panic("switch");

  lock.lock_noipl();

  pr_warn("add a thread reaper!\n");

  state = ThreadState::Terminating;

  Scheduler::for_this_cpu().yield(this);
  __builtin_unreachable();
}

extern "C" [[noreturn]] void __thread_exit_trampoline() {
  Thread::Current()->exit();
}

} // namespace yak

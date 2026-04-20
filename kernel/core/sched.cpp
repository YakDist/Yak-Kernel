/*
https://web.cs.ucdavis.edu/~roper/ecs150/ULE.pdf

From my rough understanding we have three classes:
- idle
- time-shared
- real-time

We have three different queues: idle, current, next

idle queue is only ran once we don't have any thread whatsover on any other
queue time-shared threads may not preempt any other thread real-time threads may
preempt lower priority threads

Threads may be deemed interactive and end up on current queues too / handled as
realtime threads

i'll implement everything very primitively. interactivity and other features
will follow later on :^)
*/

#include <assert.h>
#include <frg/mutex.hpp>
#include <yak/arch-intr.h>
#include <yak/arch.h>
#include <yak/cpudata.h>
#include <yak/ipl.h>
#include <yak/log.h>
#include <yak/panic.h>
#include <yak/percpu.h>
#include <yak/ps.h>
#include <yak/sched.h>
#include <yak/sched_prio.h>
#include <yak/softint.h>
#include <yak/thread.h>

#ifdef x86_64
#include <x86_64/cpu_features.h>
#endif

namespace yak {
void Scheduler::init(CpuData *cpu, Thread *idle_thread) {
  idle_thread->state = ThreadState::Running;
  cpu->current_thread = idle_thread;
  cpu->kernel_stack_top = idle_thread->kernel_stack_top;

  cpu->sched.initialize(cpu, idle_thread);

  auto sched = cpu->sched.get();
  sched->idle_thread_ = idle_thread;
}

Scheduler &Scheduler::for_this_cpu() {
  return *CpuData::Current()->sched;
}

// Both scheduler and thread shall be locked upon entry
void Scheduler::insert(Thread *thread, bool remote) {
  while (true) {
    thread->last_cpu = sched_cpu_;
    thread->state = ThreadState::Queued;

    auto current = next_thread_ ? next_thread_ : sched_cpu_->current_thread;

    if (sched_prio::is_real_time(thread->priority)) {
      if (thread->priority <= current->priority) {
        // thread's priority is not high enough to preempt
        rr_queue_.insert(thread);
        return;
      }
    } else {
      // Check if current is either
      // 1) the idle thread
      // 2) a thread of the Idle class
      bool can_preempt = (current->priority == SchedPrio::Idle &&
                          thread->priority > SchedPrio::Idle);
      if (!can_preempt) {
        // Anything not real time prio cannot preempt anything
        if (sched_prio::is_time_share(thread->priority)) {
          // TODO: advanced insertion stuff
          // do not starve the time share threads completely and so on
          rr_queue_.insert(thread);
        } else {
          idle_queue_.push_back(thread);
        }

        return;
      }
    }

    // We can preempt the currently running thread!

    thread->state = ThreadState::WaitingForSwitch;

    // nullptr if none
    auto evicted = next_thread_;

    next_thread_ = thread;

    if (evicted) {
      // reinsert the old thread
      thread = evicted;
      // go through the whole thing again
      // we merely replaced the next thread
      // so the loop will simply return
      continue;
    }

    // now! we can finally preempt the currently running thread!
    if (remote) {
      softint_issue_other(sched_cpu_, Ipl::dispatch);
    } else {
      softint_issue(Ipl::dispatch);
    }

    return;
  }
}

Thread *Scheduler::select_next(SchedPrio min_priority) {
  if (!rr_queue_.empty()) {
    auto ceil = rr_queue_.priority_ceil();
    if (min_priority > ceil)
      return nullptr;
    auto t = rr_queue_.pop(ceil);
    assert(t);
    return t;
  } else if (!idle_queue_.empty()) {
    return idle_queue_.pop_front();
  }

  return nullptr;
}

void Scheduler::resume_locked(Thread *thread) {
  assert(thread->lock.is_locked());

  auto cpu = thread->affinity_cpu;
  if (cpu == nullptr) {
    cpu = CpuData::Current();
  }

  auto guard = frg::guard(&lock_);
  insert(thread, cpu != CpuData::Current());
}

void Scheduler::resume(Thread *thread) {
  auto guard = frg::guard(&thread->lock);
  resume_locked(thread);
}

static inline void wait_for_switch(Thread *thread) {
  while (thread->is_switching.load(std::memory_order_acquire))
    busyloop_hint();
}

extern "C" [[gnu::no_instrument_function]]
void sched_finalize_switch(Thread *current, Thread *next) {
  current->is_switching.store(false, std::memory_order_release);
  current->lock.unlock();
  next->state = ThreadState::Running;
}

[[gnu::no_instrument_function]]
static void do_switch(Thread *current, Thread *thread) {
  assert(iplget() == Ipl::dispatch);
  assert(current && thread);
  assert(current != thread);
  assert(current->lock.is_locked());
  // The thread lock can be locked legally:
  // if we come from shed_yield and we have waited for is_switching = false
  // successfully, the spinlock is unlocked only afterwards

  assert(current->state != ThreadState::Terminating ||
         thread->state != ThreadState::Undefined ||
         current->state != ThreadState::Blocked);

  current->affinity_cpu = CpuData::Current();

  if (thread->is_user) {
    if (current->effective_process != thread->effective_process) {
      panic("activate other user thread process map");
    }
  }

  CPUDATA_STORE(current_thread, thread);
  CPUDATA_STORE(kernel_stack_top, thread->kernel_stack_top);

  arch::sched_switch(current, thread);

  // we should be back now
  assert(current == CPUDATA_LOAD(current_thread));
  assert(current->state != ThreadState::Terminating);
}

void Scheduler::commit_reschedule() {
  assert(iplget() == Ipl::dispatch);

  lock_.lock_noipl();

  auto next = next_thread_;
  if (next == nullptr) {
    lock_.unlock_noipl();
    return;
  }

  next_thread_ = nullptr;
  next->state = ThreadState::Switching;

  lock_.unlock_noipl();

  wait_for_switch(next);

  auto current = CPUDATA_LOAD(current_thread);
  current->lock.lock_noipl();

  // in the process of switching off the stack
  current->is_switching.store(true, std::memory_order_relaxed);

  if (current != idle_thread_) {
    lock_.lock_noipl();
    insert(current, false);
    lock_.unlock_noipl();
  } else {
    // the idle thread remains in a ready state
    current->state = ThreadState::Queued;
  }

  do_switch(current, next);
}

void Scheduler::yield(Thread *current) {
  assert(current);
  assert(current->lock.is_locked());

  lock_.lock_noipl();

  // anything is fine now
  auto next = next_thread_;
  if (next) {
    next_thread_ = nullptr;
    next->state = ThreadState::Switching;
  } else {
    next = select_next(SchedPrio{0});
  }

  lock_.unlock_noipl();

  if (next) {
    wait_for_switch(next);
    do_switch(current, next);
  } else {
    do_switch(current, idle_thread_);
  }
}

[[noreturn]]
void Scheduler::idle_loop() {
  iplx(Ipl::passive);

  while (true) {
    assert(iplget() == Ipl::passive);

    pr_debug("core %zu is idle!\n", CPUDATA_LOAD(id));

    arch::enable_interrupts();

#if defined x86_64
    // TODO: implement monitor/mwait, proper idle driver?
    asm volatile("hlt");
#else
#error "Port idle_loop to this architecture!"
#endif
  }
}

} // namespace yak

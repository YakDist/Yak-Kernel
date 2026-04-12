#include <assert.h>
#include <exception>
#include <frg/mutex.hpp>
#include <new>
#include <yak/arch-intr.h>
#include <yak/cpudata.h>
#include <yak/ipl.h>
#include <yak/panic.h>
#include <yak/percpu.h>
#include <yak/sched.h>
#include <yak/thread.h>

namespace yak {
void Scheduler::init(CpuData *cpu) {
  new (&cpu->sched) Scheduler();
}

Scheduler &Scheduler::for_this_cpu() {
  return get_cpu_self().sched;
}

void Scheduler::insert(Thread *thread, bool remote) {
  while (true) {
    thread->last_cpu = sched_cpu;
    thread->state = ThreadState::Queued;

    auto next = next_thread;
    auto comp = next;
    if (!comp)
      comp = sched_cpu->current_thread;

    if (sched_prio::is_real_time(thread->priority)) {
      if (thread->priority <= comp->priority) {
        // thread's priority is not high enough to preempt
        rr_queue.insert(thread);
        return;
      }
    } else {
      if (comp != idle_thread) {
        // Anything not real time prio cannot preempt
        if (sched_prio::is_time_share(thread->priority)) {
          // TODO: advanced insertion stuff
          // do not starve the time share threads completely and so on
          rr_queue.insert(thread);
        } else {
          idle_queue.push_back(thread);
        }

        return;
      }

      // We can preempt the idle thread!
    }

    thread->state = ThreadState::WaitingForSwitch;
    next_thread = thread;

    if (next) {
      // reinsert the old thread
      thread = next;
      // go through the whole thing again
      // we merely replaced the next thread
      // so the loop will simply return
      continue;
    } else {
      // now! we can actually preempt the currently running thread!
      // therefor actually cause a switch
      if (remote) {
        panic("remote softint");
      } else {
        panic("local softint");
      }
    }
  }
}

Thread *Scheduler::select_next(SchedPrio min_priority) {
  if (!rr_queue.empty()) {
    auto ceil = rr_queue.priority_ceil();
    if (min_priority > ceil)
      return nullptr;
    auto t = rr_queue.pop(ceil);
    assert(t);
    return t;
  } else if (!idle_queue.empty()) {
    return idle_queue.pop_front();
  }

  return nullptr;
}

void Scheduler::resume_locked(Thread *thread) {
  assert(thread->lock.is_locked());

  auto cpu = thread->affinity_cpu;
  if (!cpu) {
    cpu = &get_cpu_self();
  }

  auto guard = frg::guard(&lock);
  insert(thread, cpu == &get_cpu_self());
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
void scheduler_finalize_switch(Thread *current, Thread *next) {
  current->is_switching.store(false, std::memory_order_release);
  current->lock.unlock();
  next->state = ThreadState::Running;
}

[[gnu::no_instrument_function]]
static void do_switch(Thread *current, Thread *thread) {
}

void Scheduler::yield(Thread *current) {
  assert(current);
  assert(current->lock.is_locked());
  auto guard = frg::guard(&lock);
  // anything is fine now
  auto next = next_thread;
  if (next) {
    next_thread = nullptr;
    next->state = ThreadState::Switching;
  } else {
    next = select_next(SchedPrio{0});
  }

  guard.unlock();

  if (next) {
    wait_for_switch(next);
    panic("switch to next thread");
  } else {
    panic("switch to idle thread");
  }
}

static void idle_loop() {
  iplx(Ipl::passive);
  while (true) {
    assert(iplget() == Ipl::passive);
    arch::enable_interrupts();
#if defined x86_64
    asm volatile("hlt");
#else
#error "Port idle_loop to this architecture!"
#endif
  }
}

} // namespace yak

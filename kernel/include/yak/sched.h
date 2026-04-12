#pragma once

#include <cstdint>
#include <utility>
#include <yak/math.h>
#include <yak/sched_prio.h>
#include <yak/thread.h>

namespace yak {

struct RunQueue {
  uint64_t ready_mask = 0;
  ThreadQueue queues[sched_prio::PRIO_COUNT];

  inline void insert(Thread *thread) {
    auto prio = std::to_underlying(thread->priority);
    auto &queue = queues[prio];
    queue.push_back(thread);
    ready_mask |= (1ULL << prio);
  }

  inline void remove(Thread *thread) {
    auto prio = std::to_underlying(thread->priority);
    auto &queue = queues[prio];
    queue.erase(thread);
    if (queue.empty()) {
      ready_mask &= ~(1ULL << prio);
    }
  }

  inline Thread *pop(SchedPrio sprio) {
    auto prio = std::to_underlying(sprio);
    auto &queue = queues[prio];
    auto t = queue.pop_front();
    if (queue.empty()) {
      ready_mask &= ~(1ULL << prio);
    }
    return t;
  }

  inline SchedPrio priority_ceil() const {
    auto ceil = ilog2(ready_mask);
    return SchedPrio(ceil);
  }

  inline bool empty() const {
    return ready_mask == 0;
  }

  inline bool has_thread(SchedPrio priority) const {
    auto under = std::to_underlying(priority);
    if (ready_mask & (1ULL << under))
      return true;
    return false;
  }
};

class Scheduler {
public:
  static void init(CpuData *cpu);
  static Scheduler &for_this_cpu();

  void insert(Thread *thread, bool remote);

  void resume_locked(Thread *thread);
  void resume(Thread *thread);

  void yield(Thread *current);

  Thread *select_next(SchedPrio priority);

private:
  IplSpinlock lock;

  RunQueue rr_queue;
  ThreadQueue idle_queue;

  CpuData *sched_cpu;

  Thread *idle_thread;
  Thread *next_thread;
};

} // namespace yak

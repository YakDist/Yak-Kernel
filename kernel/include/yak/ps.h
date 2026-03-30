#pragma once

#include "yak/spinlock.h"
#include <frg/intrusive.hpp>
#include <frg/list.hpp>

namespace yak {

constexpr size_t PID_MAX_LIMIT = 32768;
using pid_t = int;

enum class ProcessState {
  // The default state
  PROCESS_ALIVE,
  // The process has no threads left
  PROCESS_ZOMBIE,
};

class Process {
public:
  Process(Process *parent = nullptr);

  ProcessState state;

  pid_t pid;

  IplSpinLock childlist_lock;
  frg::default_list_hook<Process> childlist_hook;
  frg::intrusive_list<
      Process, frg::locate_member<Process, frg::default_list_hook<Process>,
                                  &Process::childlist_hook>>
      childlist;
};

// kernel 'pseudo-process'
extern Process kernel_process;

} // namespace yak

#include <algorithm>
#include <string.h>
#include <yak/thread.h>

namespace yak {
Thread::Thread(frg::string_view name, unsigned int initial_priority,
               Process *parent_process, bool user_thread)
    : state{ThreadState::Undefined},
      priority{initial_priority},
      parent_process{parent_process},
      effective_process{parent_process},
      is_user{user_thread} {
  auto name_copy_len = std::min(name.size() - 1, THREAD_MAX_NAME_LEN - 1);
  memcpy(this->name, name.data(), name_copy_len);
}
} // namespace yak

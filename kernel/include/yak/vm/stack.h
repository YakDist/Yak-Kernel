#pragma once

#include <yak/vm/address.h>

namespace yak {
/* returns stack TOP */
vaddr_t kstack_alloc();
/* takes stack TOP */
void kstack_free(vaddr_t stack_top);
} // namespace yak

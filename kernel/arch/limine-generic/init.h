#pragma once

namespace yak::limine {
void mem_init();
void mem_reclaim();

[[gnu::weak]]
void post_memblock();
} // namespace yak::limine

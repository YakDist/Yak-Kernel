#pragma once

namespace yak::limine {
void mem_init();

[[gnu::weak]]
void post_memblock();
} // namespace yak::limine

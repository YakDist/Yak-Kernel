#pragma once

#include <atomic>
#include <bit>
#include <climits>
#include <cstddef>

namespace yak {

using bitset_word_t = unsigned long;
inline constexpr size_t BITSET_WORD_BITS = sizeof(bitset_word_t) * CHAR_BIT;

namespace bitset_detail {

constexpr size_t word_idx(size_t bit) { return bit / BITSET_WORD_BITS; }

constexpr size_t bit_idx(size_t bit) { return bit & (BITSET_WORD_BITS - 1); }

constexpr bitset_word_t mask(size_t bit) {
  return bitset_word_t(1UL) << bit_idx(bit);
}

} // namespace bitset_detail

template <size_t NBits> struct Bitset {
  static constexpr size_t WORD_COUNT =
      (NBits + BITSET_WORD_BITS - 1) / BITSET_WORD_BITS;

  bitset_word_t words[WORD_COUNT];

  void init() {
    for (size_t i = 0; i < WORD_COUNT; i++)
      words[i] = 0;
  }

  void set(size_t bit) {
    words[bitset_detail::word_idx(bit)] |= bitset_detail::mask(bit);
  }
  void clear(size_t bit) {
    words[bitset_detail::word_idx(bit)] &= ~bitset_detail::mask(bit);
  }
  bool test(size_t bit) const {
    return (words[bitset_detail::word_idx(bit)] & bitset_detail::mask(bit)) !=
           0;
  }

  template <typename Fn> void for_each_set_bit(Fn f) const {
    for (size_t w = 0; w < WORD_COUNT; w++) {
      bitset_word_t tmp = words[w];
      while (tmp != 0) {
        size_t bit = w * BITSET_WORD_BITS + std::countr_zero(tmp);
        f(bit);
        tmp &= tmp - 1; // clear lowest set bit
      }
    }
  }
};

template <size_t NBits> struct AtomicBitset {
  static constexpr size_t WORD_COUNT =
      (NBits + BITSET_WORD_BITS - 1) / BITSET_WORD_BITS;

  std::atomic<bitset_word_t> words[WORD_COUNT];

  AtomicBitset() {
    for (auto &w : words)
      w.store(0, std::memory_order_relaxed);
  }

  AtomicBitset(const AtomicBitset &) = delete;
  AtomicBitset &operator=(const AtomicBitset &) = delete;

  void set(size_t bit) {
    words[bitset_detail::word_idx(bit)].fetch_or(bitset_detail::mask(bit),
                                                 std::memory_order_release);
  }

  void clear(size_t bit) {
    words[bitset_detail::word_idx(bit)].fetch_and(~bitset_detail::mask(bit),
                                                  std::memory_order_release);
  }

  bool test(size_t bit) const {
    return (words[bitset_detail::word_idx(bit)].load(
                std::memory_order_acquire) &
            bitset_detail::mask(bit)) != 0;
  }

  bool compare_exchange_bit(size_t bit, bool expected, bool desired) {
    size_t idx = bitset_detail::word_idx(bit);
    bitset_word_t mask = bitset_detail::mask(bit);

    auto &word = words[idx];

    bitset_word_t old = word.load(std::memory_order_acquire);

    while (true) {
      bool current = (old & mask) != 0;

      if (current != expected)
        return false;

      bitset_word_t new_val;
      if (desired)
        new_val = old | mask; // set bit
      else
        new_val = old & ~mask; // clear bit

      // Try CAS
      if (word.compare_exchange_weak(old, new_val, std::memory_order_acq_rel,
                                     std::memory_order_acquire)) {
        return true;
      }

      // CAS failed
    }
  }

  template <typename Fn> void for_each_set_bit(Fn f) const {
    for (size_t w = 0; w < WORD_COUNT; w++) {
      bitset_word_t tmp = words[w].load(std::memory_order_acquire);
      while (tmp != 0) {
        size_t bit = w * BITSET_WORD_BITS + std::countr_zero(tmp);
        f(bit);
        tmp &= tmp - 1; // clear lowest set bit
      }
    }
  }
};

} // namespace yak

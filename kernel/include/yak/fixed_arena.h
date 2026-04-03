#pragma once

#include <cstddef>
#include <span>
#include <yak/panic.h>

namespace yak {
template <typename T, size_t N> class FixedArena {
public:
  T &alloc_slot() {
    if (m_count >= N)
      panic("fixed arena: capacity exceeded");
    return m_data[m_count++];
  }

  std::span<T> span() {
    return {m_data.data(), m_count};
  }
  std::span<const T> span() const {
    return {m_data.data(), m_count};
  }

  T *data() {
    return m_data.data();
  }
  const T *data() const {
    return m_data.data();
  }

  T &operator[](size_t i) {
    return m_data[i];
  }
  const T &operator[](size_t i) const {
    return m_data[i];
  }

  size_t size() const {
    return m_count;
  }
  bool empty() const {
    return m_count == 0;
  }

private:
  std::array<T, N> m_data{};
  size_t m_count = 0;
};

} // namespace yak

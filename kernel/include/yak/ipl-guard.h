#pragma once

#include <yak/ipl.h>

namespace yak {
class IplGuard {
public:
  explicit IplGuard(Ipl ipl) : saved_(iplr(ipl)) {}

  ~IplGuard() {
    iplx(saved_);
  }

  IplGuard(const IplGuard &) = delete;
  IplGuard &operator=(const IplGuard &) = delete;
  IplGuard(IplGuard &&) = delete;
  IplGuard &operator=(IplGuard &&) = delete;

private:
  Ipl saved_;
};
} // namespace yak

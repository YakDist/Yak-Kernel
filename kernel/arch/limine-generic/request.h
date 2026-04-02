#pragma once

#include <yak/panic.h>

#define LIMINE_REQ [[gnu::used, gnu::section(".limine_requests")]]

namespace yak::limine {
template <typename Request>
auto &ensure_request(Request &req, const char *msg) {
  if (!req.response)
    panic(msg);
  return *req.response;
}
} // namespace yak::limine

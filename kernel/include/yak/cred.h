#pragma once

namespace yak {
struct Cred {
  int uid, euid;
  int gid, egid;
};
} // namespace yak

#include <yak/numa.h>

namespace yak {

static int next_id = 0;

auto g_domains = FixedArena<Domain, MAX_AFFINITIES>();

Domain &Domain::from_id(unsigned int id) {
  return g_domains[id];
}

Domain &Domain::from_firmware_id(unsigned int firmware_id) {
  for (auto &dom : g_domains.span()) {
    if (dom.firmware_id == firmware_id)
      return dom;
  }

  auto &new_dom = g_domains.alloc_slot();
  new_dom.id = next_id++;
  new_dom.firmware_id = firmware_id;
  return new_dom;
}

} // namespace yak

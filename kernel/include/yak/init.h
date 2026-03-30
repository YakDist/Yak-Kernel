#pragma once

#include <initgraph.h>

namespace yak {

struct GlobalInitEngine : initgraph::Engine {};

extern GlobalInitEngine init_engine;

} // namespace yak

#pragma once

#include <yio/Service.hh>
#include <yio/EventSource.hh>
#include <yakpp/meta.hh>
#include <yakpp/Object.hh>

namespace yak::io
{
class InterruptEventSource : EventSource {
	IO_OBJ_DECLARE(InterruptEventSource);

    public:
	static InterruptEventSource *
	interruptEventSource(yak::Object *owner, Action action,
			     Service *provider = nullptr, int intSources = 0);
};
}

#pragma once

#include <yio/Interrupt.h>
#include <yio/Service.hh>

namespace yak::io
{

struct InterruptVector {
	Service *nub;
};

class InterruptController : Service {
	IO_OBJ_DECLARE(InterruptController);

    public:
	virtual bool registerInterrupt(int source, Service *nub,
				       InterruptHandler handler, void *refCon);

    protected:
	InterruptVector *vectors;
};

}

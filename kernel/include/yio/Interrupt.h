#pragma once

typedef void (*InterruptHandler)(void *target, void *refCon, void *nub,
				 int source);

#include "yak/panic.h"
#include <yak/wait.h>
#include <yak/sched.h>
#include <yakpp/meta.hh>
#include <yakpp/Event.hh>
#include <yio/WorkLoop.hh>
#include <yak/log.h>
#include <yak/cpudata.h>

namespace yak::io
{

IO_OBJ_DEFINE(WorkLoop, Object);
#define super Object

void WorkLoop::threadMain()
{
	while (true) {
		wakeEvent_.wait();
	}
}

static void enterWorkLoopMain(void *ctx)
{
	auto loop = static_cast<WorkLoop *>(ctx);
	loop->threadMain();
	sched_exit_self();
}

void WorkLoop::init()
{
	gateLock_.init("gateLock");

	queueLock_.init();

	wakeEvent_.init(false, Event::kEventSync);

	kernel_thread_create("workloop", SCHED_PRIO_TIME_SHARE_END,
			     enterWorkLoopMain, this, true, &workThread_);
}

WorkLoop *WorkLoop::workLoop()
{
	WorkLoop *wl;
	ALLOC_INIT(wl, WorkLoop);
	return wl;
}

bool WorkLoop::onThread()
{
	return curthread() == workThread_;
}

void WorkLoop::lockGate()
{
	if (gateLock_.isOwner()) {
		// Current thread owns the lock
		recursiveCount_++;
		return;
	}

	// Acquire the lock for the first time
	gateLock_.lock();
	recursiveCount_ = 1;
}

void WorkLoop::unlockGate()
{
	if (!gateLock_.isOwner()) {
		panic("unlockGate from non-owner!\n");
	}

	if (--recursiveCount_ == 0) {
		gateLock_.unlock();
	}
}

}

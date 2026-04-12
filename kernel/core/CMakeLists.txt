add_subdirectory(irq)
add_subdirectory(syscall)

yak_add_sources(
	cpudata.c
	init.c
	init_node.c
	elf.c
	ringbuffer.c
	kevent.c
	initrd.c
	panic.c
	kinfo.c
	object.c
	status.c
	subr_tree.c
	hashtable.c
	printk.c
	root.c
	rwlock.c
	semaphore.c
	spinlock.c
	mutex.c
	jobctl.c
	file.c
	timer.c
	console.c
	symbol.c
	clocksource.c
	launch.c
	process.c
	sched.c
	thread.c
	wait.c
)

if (CONFIG_BOOT_BANNER)
	yak_add_sources(banner.c)
endif ()

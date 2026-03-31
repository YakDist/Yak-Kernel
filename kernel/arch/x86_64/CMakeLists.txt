set(YAK_INCLUDE_LIMINE ON PARENT_SCOPE)
set(YAK_INCLUDE_ACPI ON PARENT_SCOPE)

yak_add_sources(
	start.S
	early_log.cpp
	init.cpp
	idt.cpp
	idt.S
	gdt.cpp
	address_space.cpp
	syscall.cpp
	ipl.cpp
)

yak_compile_options(
	-m64
	-march=x86-64
	$<$<CXX_COMPILER_ID:Clang>:-target x86_64-unknown-none-elf>
	-mabi=sysv
	-mno-red-zone
	-mno-sse
	-mno-sse2
	-mno-mmx
	-mno-80387
	-mcmodel=kernel
)

yak_link_options(
	-T${CMAKE_CURRENT_LIST_DIR}/kernel.ld
	-zmax-page-size=0x1000
)

yak_link_depends(
	${CMAKE_CURRENT_LIST_DIR}/kernel.ld
)

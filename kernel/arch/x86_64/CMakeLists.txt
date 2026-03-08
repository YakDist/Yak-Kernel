yak_add_sources(
	start.S
	early_log.cpp
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
	-Wl,-zmax-page-size=0x1000
)

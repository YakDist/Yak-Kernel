set(YAK_INCLUDE_LIMINE ON PARENT_SCOPE)
set(YAK_INCLUDE_ACPI ON PARENT_SCOPE)

yak_add_sources(
	start.S
	ipl.cpp
	init.cpp
	sched.cpp
	sched.S
	early_log.cpp
)

yak_compile_options(
	-march=rv64imac_zicsr_zifencei
	-mabi=lp64
	-mno-relax
)

yak_link_options(
	-T${CMAKE_CURRENT_LIST_DIR}/kernel.ld
	-zmax-page-size=0x1000
	-m elf64lriscv
	--no-relax
)

yak_link_depends(
	${CMAKE_CURRENT_LIST_DIR}/kernel.ld
)

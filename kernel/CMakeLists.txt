add_executable(yak)

include(symbols.cmake)

find_package(PkgConfig REQUIRED)

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _processor)
if (_processor STREQUAL "x86_64")
	set(YAK_ARCH "x86_64" CACHE STRING "Project architecture" FORCE)
else()
	message(FATAL_ERROR
		"Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}"
	)
endif()

pkg_check_modules(FREESTND_C_HDRS REQUIRED IMPORTED_TARGET freestnd-c-hdrs-x86_64)
pkg_check_modules(FREESTND_CXX_HDRS REQUIRED IMPORTED_TARGET freestnd-cxx-hdrs-x86_64)
pkg_check_modules(FRIGG REQUIRED IMPORTED_TARGET frigg)

add_subdirectory(arch/${YAK_ARCH})
add_subdirectory(core)
add_subdirectory(lib)

yak_add_includes(
	include
	arch
)

yak_defines(
	__KERNEL__
	${YAK_ARCH}
)

yak_compile_options(
	-fstack-protector-strong
	-fstack-check
	-mstack-protector-guard=global
)

yak_compile_options(
	-ffreestanding
	-nostdinc
	-fno-omit-frame-pointer
	-fno-strict-aliasing
	-mgeneral-regs-only
	-pipe
	-fdata-sections
	-ffunction-sections
	-fno-pie
	-fno-pic
	-Wall
	-Wextra
	$<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
	$<$<CXX_COMPILER_ID:Clang>:-fstrict-vtable-pointers>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
	$<$<COMPILE_LANGUAGE:CXX>:-fcheck-new>
)

yak_link_options(
	-nostdlib
	-static
	-no-pie
	-Wl,-gc-sections
)

yak_link(
	PkgConfig::FRIGG
	PkgConfig::FREESTND_C_HDRS
	PkgConfig::FREESTND_CXX_HDRS
)

install(TARGETS yak DESTINATION share/yak)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/yak.sym DESTINATION share/yak)

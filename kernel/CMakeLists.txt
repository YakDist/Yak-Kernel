add_executable(yak)

include(symbols.cmake)
include(build.cmake)

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

find_package(Git QUIET)

add_custom_target(update_version_header ALL
	COMMAND ${CMAKE_COMMAND} 
	-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} 
	-DBINARY_DIR=${CMAKE_BINARY_DIR} 
	-DVERSION_STR=${PROJECT_VERSION}
	-DVERSION_MAJOR=${PROJECT_VERSION_MAJOR}
	-DVERSION_MINOR=${PROJECT_VERSION_MINOR}
	-DVERSION_PATCH=${PROJECT_VERSION_PATCH}
	-P "${CMAKE_CURRENT_SOURCE_DIR}/get_git_hash.cmake"
	BYPRODUCTS "${CMAKE_BINARY_DIR}/generated/include/yak/version.h"
)

add_dependencies(yak update_version_header)

add_subdirectory(arch/${YAK_ARCH})
add_subdirectory(core)
add_subdirectory(lib)

yak_add_includes(
	include
	arch
)

target_include_directories(yak PRIVATE
	# Kconfig
	${AUTOCONF_DIR}
	# Version
	${CMAKE_BINARY_DIR}/generated/include
)

yak_defines(
	__KERNEL__
	${YAK_ARCH}
)

if (CONFIG_STACK_CANARIES)
	yak_compile_options(
	-fstack-protector-strong
	-fstack-check
	-mstack-protector-guard=global
	)
else()
	yak_compile_options(-fno-stack-protector)
endif()

if (CONFIG_LTO)
	target_enable_lto(yak)
endif()

if (CONFIG_UBSAN)
	yak_compile_options(
		-fsanitize=undefined
	)
endif()

yak_compile_options(
	-ffreestanding
	-nostdinc
	-fno-omit-frame-pointer
	-fno-strict-aliasing
	-fno-semantic-interposition
	-fvisibility=hidden
	-mgeneral-regs-only
	-pipe
	-fdata-sections
	-ffunction-sections
	-fno-pie
	-fno-pic
	-fno-plt
	-Wall
	-Wextra
	-Wpedantic
	-Wshadow
	#-Wconversion
	-Wnull-dereference
	$<$<CXX_COMPILER_ID:Clang>:-Wthread-safety>
	-Wformat=2
	$<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
	$<$<COMPILE_LANGUAGE:CXX>:-fcheck-new>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
	$<$<CXX_COMPILER_ID:Clang>:-fstrict-vtable-pointers>
)

yak_link_options(
	-nostdlib
	-static
	-Wl,-gc-sections
)

yak_link(
	PkgConfig::FRIGG
	PkgConfig::FREESTND_C_HDRS
	PkgConfig::FREESTND_CXX_HDRS
)

install(TARGETS yak DESTINATION share/yak)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/yak.sym DESTINATION share/yak)

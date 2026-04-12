add_executable(yak)

include(symbols.cmake)
include(build.cmake)

find_package(PkgConfig REQUIRED)
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

include(${VENDOR_DIR}/flanterm.cmake)
include(${VENDOR_DIR}/nanoprintf.cmake)
include(${VENDOR_DIR}/cc-runtime.cmake)
include(${VENDOR_DIR}/pairing-heap.cmake)
include(${VENDOR_DIR}/freestnd-c-hdrs.cmake)
include(${VENDOR_DIR}/freestnd-cxx-hdrs.cmake)

add_subdirectory(arch)
add_subdirectory(lib)
add_subdirectory(vm)
add_subdirectory(fs)

add_subdirectory(core)
add_subdirectory(core-cpp)
add_subdirectory(io)

yak_add_includes(
	include
	arch
	arch/${YAK_ARCH}/include
)

target_include_directories(yak PRIVATE
	# Contains version.h
	${CMAKE_BINARY_DIR}/generated/include
	# Contains config.h
	${CMAKE_BINARY_DIR}/kconfig/include/generated
)

yak_defines(
	__KERNEL__
	ARCH_STR="${YAK_ARCH}"
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

if (CONFIG_UBSAN)
	yak_compile_options(
		-fsanitize=undefined
	)
endif()

if (CONFIG_LTO)
	target_enable_lto(yak)
endif()

if (CONFIG_BUILD_TYPE_DEBUG)
	yak_compile_options(-O0)
elseif(CONFIG_BUILD_TYPE_RELEASE)
	yak_compile_options(-O2)
elseif(CONFIG_BUILD_TYPE_MINSIZEREL)
	yak_compile_options(-Os)
elseif(CONFIG_BUILD_TYPE_FASTFLAGSREL)
	yak_compile_options(-O3)
endif()

yak_compile_options(
	-ffile-prefix-map=${PROJECT_ROOT_NORMALIZED}=./
	-fmacro-prefix-map=${PROJECT_ROOT_NORMALIZED}=./
)

yak_compile_options(
	-ffreestanding
	-ggdb3
	-nostdinc
	-fno-omit-frame-pointer
	-fno-strict-aliasing
	-fno-semantic-interposition
	-fvisibility=hidden
	-fasynchronous-unwind-tables
	-mgeneral-regs-only
	-pipe
	-fdata-sections
	-ffunction-sections
	-fno-pie
	-fno-pic
	-fno-plt
	-Wall
	-Wextra
	-Wshadow
	-Wimplicit-fallthrough
	#-Wconversion
	-Wnull-dereference
	$<$<CXX_COMPILER_ID:Clang>:-Wthread-safety>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
	$<$<COMPILE_LANGUAGE:CXX>:-fcheck-new>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
	$<$<CXX_COMPILER_ID:Clang>:-fstrict-vtable-pointers>
)

yak_link_options(
	-nostdlib
	--eh-frame-hdr
	-gc-sections
)

target_compile_definitions(yak
    PUBLIC
	UACPI_NATIVE_ALLOC_ZEROED=1
	FLANTERM_FB_DISABLE_BUMP_ALLOC=1
)

install(TARGETS yak DESTINATION share/yak)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/yak.sym DESTINATION share/yak)

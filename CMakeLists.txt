cmake_minimum_required(VERSION 3.21)

set(YAK_ARCH "" CACHE STRING "Kernel architecture")

if (NOT YAK_ARCH STREQUAL "x86_64")
	message(FATAL_ERROR
		"Unsupported architecture: ${YAK_ARCH}"
	)
endif()

set(YAK_TOOLCHAIN "clang" CACHE STRING "Kernel toolchain")

set(YAK_TOOLCHAIN_DIR "${CMAKE_SOURCE_DIR}/toolchain")
include("${YAK_TOOLCHAIN_DIR}/toolchain_${YAK_TOOLCHAIN}.cmake")

project(
	Yak
	DESCRIPTION "Yet Another [unix] Kernel"
	VERSION 0.0.0
	HOMEPAGE_URL https://codeberg.org/YakDist/Yak
	LANGUAGES ${YAK_LANGUAGES}
)

set(CMAKE_BUILD_TYPE "None" CACHE STRING "" FORCE)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})
cmake_path(NORMAL_PATH PROJECT_ROOT OUTPUT_VARIABLE PROJECT_ROOT_NORMALIZED)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_ROOT}/cmake)


set(AUTOCONF_H ${CMAKE_BINARY_DIR}/generated/include/yak/config.h)

include(kconfig)

add_subdirectory(kernel)

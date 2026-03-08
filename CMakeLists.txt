cmake_minimum_required(VERSION 3.21)

project(
	Yak
	DESCRIPTION "Yet Another [unix] Kernel"
	VERSION 0.0.0
	HOMEPAGE_URL https://codeberg.org/YakDist/Yak
	LANGUAGES C CXX ASM
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})
list(APPEND CMAKE_MODULE_PATH ${PROJECT_ROOT}/cmake)

include(kconfig)

add_subdirectory(kernel)

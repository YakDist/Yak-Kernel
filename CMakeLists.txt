cmake_minimum_required(VERSION 3.21)

project(
	Yak
	VERSION 0.0.0
	HOMEPAGE_URL https://codeberg.org/YakDist/Yak
	LANGUAGES C CXX ASM
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

function(yak_add_sources)
	foreach(SOURCE_FILE ${ARGN})
		target_sources(yak PRIVATE "${CMAKE_CURRENT_LIST_DIR}/${SOURCE_FILE}")
	endforeach()
endfunction()

function(yak_add_includes)
	foreach(DIR ${ARGN})
		target_include_directories(yak PRIVATE "${CMAKE_CURRENT_LIST_DIR}/${DIR}")
	endforeach()
endfunction()

function(yak_compile_options)
	target_compile_options(yak PRIVATE ${ARGN})
endfunction()

function(yak_link_options)
	target_link_options(yak PRIVATE ${ARGN})
endfunction()

function(yak_defines)
	target_compile_definitions(yak PRIVATE ${ARGN})
endfunction()

add_subdirectory(kernel)

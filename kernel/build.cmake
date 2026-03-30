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

function(yak_link_depends)
	set_target_properties(yak PROPERTIES LINK_DEPENDS ${ARGN})
endfunction()

function(yak_defines)
	target_compile_definitions(yak PRIVATE ${ARGN})
endfunction()

function(yak_link)
	target_link_libraries(yak PRIVATE ${ARGN})
endfunction()

function(target_enable_lto target)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported)

    if(NOT supported)
        return()
    endif()

    set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

	#if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		#message("Enable ThinLTO for target ${target}")
		#target_compile_options(${target} PRIVATE -flto=thin)
		#target_link_options(${target} PRIVATE -flto=thin)
	#endif()
endfunction()

add_subdirectory(${YAK_ARCH})

if (YAK_INCLUDE_LIMINE) 
	add_subdirectory(limine-generic)
endif()

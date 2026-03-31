add_subdirectory(${YAK_ARCH})

if (YAK_INCLUDE_ACPI)
	target_sources(yak PRIVATE ${UACPI_SOURCES})
	target_include_directories(yak PRIVATE ${UACPI_INCLUDES})
endif()

if (YAK_INCLUDE_LIMINE) 
	add_subdirectory(limine-generic)
endif()

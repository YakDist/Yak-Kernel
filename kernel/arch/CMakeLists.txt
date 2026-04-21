add_subdirectory(${YAK_ARCH})

if (YAK_INCLUDE_ACPI)
	include(${VENDOR_DIR}/uacpi/uacpi.cmake)
	yak_add_sources(${UACPI_SOURCES})
	yak_add_includes(${UACPI_INCLUDES})
endif()

if (YAK_INCLUDE_LIMINE) 
	yak_add_includes(${VENDOR_DIR}/limine-protocol/include)
	add_subdirectory(limine-generic)
endif()

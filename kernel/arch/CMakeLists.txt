add_subdirectory(${YAK_ARCH})

if (YAK_INCLUDE_ACPI)
	include(${VENDOR_DIR}/uacpi/uacpi.cmake)
	target_sources(yak PRIVATE ${UACPI_SOURCES})
	target_include_directories(yak PRIVATE ${UACPI_INCLUDES})
endif()

if (YAK_INCLUDE_LIMINE) 
	target_include_directories(yak PRIVATE ${VENDOR_DIR}/limine-protocol/include)
	add_subdirectory(limine-generic)
endif()

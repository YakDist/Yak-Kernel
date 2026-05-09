if (CONFIG_ACPI_ENABLED)
	add_subdirectory(acpi)

	include(${VENDOR_DIR}/uacpi/uacpi.cmake)
	yak_add_sources(${UACPI_SOURCES})
	yak_add_includes(${UACPI_INCLUDES})
endif()

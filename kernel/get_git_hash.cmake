# This script is executed at build time
execute_process(
    COMMAND git describe --always --dirty
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT GIT_HASH)
    set(GIT_HASH "unknown")
endif()

set(PROJECT_VERSION       "${VERSION_STR}")
set(PROJECT_VERSION_MAJOR "${VERSION_MAJOR}")
set(PROJECT_VERSION_MINOR "${VERSION_MINOR}")
set(PROJECT_VERSION_PATCH "${VERSION_PATCH}")
set(GIT_HASH              "${GIT_HASH}")

configure_file("${SOURCE_DIR}/include/yak/version.h.in" "${BINARY_DIR}/generated/include/yak/version.h")

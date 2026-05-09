include("${YAK_TOOLCHAIN_DIR}/toolchain_defaults.cmake")

if (APPLE)
    execute_process(
        COMMAND
        brew --prefix lld
        OUTPUT_VARIABLE
        BREW_LLD_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(LLD_LINKER "${BREW_LLD_PREFIX}/bin/ld.lld")
else ()
    set(LLD_LINKER "ld.lld")
endif ()

# Do this because cmake attempts to link against -lc and crt0
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(YAK_TARGET "${YAK_TARGET_ARCH}-unknown-elf")

if (YAK_HOSTED_MODE)
    set(YAK_TARGET "${YAK_TARGET_ARCH}-${YAK_HOSTED_SYSTEM}")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --target=${YAK_TARGET}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --target=${YAK_TARGET}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} --target=${YAK_TARGET}")
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_ASM_COMPILER clang)
if (YAK_HOSTED_MODE)
set(CMAKE_LINKER clang++)
set(CMAKE_LINKER_TYPE LLD)
else()
set(CMAKE_LINKER ${LLD_LINKER})
endif()

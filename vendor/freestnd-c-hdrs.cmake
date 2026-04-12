include(CMakePrintHelpers)

cmake_print_variables(PROJECT_SOURCE_DIR)
cmake_print_variables(CMAKE_SYSTEM_PROCESSOR)
cmake_print_variables(YAK_ARCH)

yak_add_includes(freestnd-c-hdrs/${YAK_ARCH}/include)

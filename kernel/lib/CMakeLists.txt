yak_add_sources(
	string.c
	stack.cpp
)

if (CONFIG_UBSAN)
	yak_add_sources(ubsan.cpp)
endif()

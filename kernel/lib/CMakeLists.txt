yak_add_sources(
	string.c
	stack.cpp
	assert.cpp
	abort.cpp

	vmem.cpp
)

if (CONFIG_UBSAN)
	yak_add_sources(ubsan.cpp)
endif()

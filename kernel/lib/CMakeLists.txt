yak_add_sources(
	assert.c
	stack.c
	string.c
	cplusplus.cc
)

if (CONFIG_UBSAN)
	yak_add_sources(ubsan.c)
endif ()

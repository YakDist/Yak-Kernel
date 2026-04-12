add_custom_command(TARGET yak POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} --only-keep-debug $<TARGET_FILE:yak> $<TARGET_FILE:yak>.debug
    COMMAND ${CMAKE_STRIP} --strip-debug --strip-unneeded $<TARGET_FILE:yak>
    COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=$<TARGET_FILE:yak>.debug $<TARGET_FILE:yak>
    COMMENT "Generating debug file for yak"
)

add_custom_command(TARGET yak POST_BUILD
    COMMAND ${CMAKE_NM} $<TARGET_FILE:yak>.debug > $<TARGET_FILE:yak>.sym
    COMMENT "Generating symbols file for yak"
)


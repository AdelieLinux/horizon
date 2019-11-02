set(EXEC_SRCS
        executor.cc
)
add_executable(hscript-executor ${EXEC_SRCS})
target_link_libraries(hscript-executor hscript)
install(TARGETS hscript-executor DESTINATION bin)

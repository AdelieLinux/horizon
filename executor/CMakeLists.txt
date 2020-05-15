set(EXEC_SRCS
        executor.cc
)
add_executable(hscript-executor ${EXEC_SRCS})
target_link_libraries(hscript-executor hscript)
install(TARGETS hscript-executor DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/executor.8 DESTINATION share/man/man8 RENAME hscript-executor.8)

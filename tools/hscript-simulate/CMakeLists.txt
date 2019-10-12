set(SIMULATOR_SRCS
        simulator.cc
)
add_executable(hscript-simulate ${SIMULATOR_SRCS})
target_link_libraries(hscript-simulate hscript)

install(TARGETS hscript-simulate DESTINATION bin)

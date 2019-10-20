set(SIMULATOR_SRCS
        simulator.cc
)
add_executable(hscript-simulate ${SIMULATOR_SRCS})
target_link_libraries(hscript-simulate hscript)

install(TARGETS hscript-simulate DESTINATION bin)

IF(RSPEC_EXECUTABLE)
add_test(NAME "RSpecSimulator"
    COMMAND ${RSPEC_EXECUTABLE} spec/simulator_spec.rb
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
set_property(TEST "RSpecSimulator"
    PROPERTY ENVIRONMENT "PATH=$PATH:${CMAKE_CURRENT_BINARY_DIR}")
ENDIF(RSPEC_EXECUTABLE)

IF(VALGRIND)
add_test(NAME "ValgrindSimulator"
    COMMAND ${VALGRIND_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/hscript-simulate ${CMAKE_SOURCE_DIR}/tests/fixtures/0001-basic.installfile)
ENDIF(VALGRIND)

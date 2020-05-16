set(SIMULATOR_SRCS
        simulator.cc
)
add_executable(hscript-simulate ${SIMULATOR_SRCS})
target_link_libraries(hscript-simulate hscript ${Boost_LIBRARIES})

install(TARGETS hscript-simulate DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/simulator.1 DESTINATION share/man/man1 RENAME hscript-simulate.1)

IF(RSPEC_EXECUTABLE)
add_test(NAME "RSpecSimulator"
    COMMAND ${RSPEC_EXECUTABLE} spec/simulator_spec.rb
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
set_property(TEST "RSpecSimulator"
    PROPERTY ENVIRONMENT "PATH=$ENV{PATH}:${CMAKE_CURRENT_BINARY_DIR}")
ENDIF(RSPEC_EXECUTABLE)

IF(VALGRIND)
add_test(NAME "ValgrindSimulator"
    COMMAND ${VALGRIND_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/hscript-simulate ${CMAKE_SOURCE_DIR}/tests/fixtures/0001-basic.installfile)
ENDIF(VALGRIND)

set(SIMULATOR_SRCS
        simulator.cc
)
add_executable(hscript-simulate ${SIMULATOR_SRCS})
target_link_libraries(hscript-simulate hscript)

install(TARGETS hscript-simulate DESTINATION bin)

add_test(NAME "RSpecSimulator"
    COMMAND rspec spec/simulator.rb
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
set_property(TEST "RSpecSimulator"
    PROPERTY ENVIRONMENT "PATH=$PATH:${CMAKE_CURRENT_BINARY_DIR}/tools/hscript-simulate")
IF(VALGRIND)
add_test(NAME "ValgrindSimulator"
    COMMAND valgrind ${CMAKE_CURRENT_BINARY_DIR}/tools/hscript-simulate/hscript-simulate tests/fixtures/0001-basic.installfile)
ENDIF(VALGRIND)

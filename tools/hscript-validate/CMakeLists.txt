set(VALIDATE_SRCS
	validator.cc
)
add_executable(hscript-validate ${VALIDATE_SRCS})
target_link_libraries(hscript-validate hscript)

install(TARGETS hscript-validate DESTINATION bin)

IF(RSPEC_EXECUTABLE)
add_test(NAME "RSpecValidator"
    COMMAND ${RSPEC_EXECUTABLE} spec/validator_spec.rb
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
set_property(TEST "RSpecValidator"
    PROPERTY ENVIRONMENT "PATH=$PATH:${CMAKE_CURRENT_BINARY_DIR}")
ENDIF(RSPEC_EXECUTABLE)

IF(VALGRIND)
add_test(NAME "ValgrindValidator"
    COMMAND ${VALGRIND_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/hscript-validate ${CMAKE_SOURCE_DIR}/tests/fixtures/0001-basic.installfile)
ENDIF(VALGRIND)

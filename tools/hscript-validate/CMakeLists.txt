set(VALIDATE_SRCS
	validator.cc
)
add_executable(hscript-validate ${VALIDATE_SRCS})
target_link_libraries(hscript-validate hscript)

install(TARGETS hscript-validate DESTINATION bin)

add_test(NAME "RSpecValidator"
    COMMAND rspec spec/validator_spec.rb
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
set_property(TEST "RSpecValidator"
    PROPERTY ENVIRONMENT "PATH=$PATH:${CMAKE_CURRENT_BINARY_DIR}")
IF(VALGRIND)
add_test(NAME "ValgrindValidator"
    COMMAND valgrind ${CMAKE_CURRENT_BINARY_DIR}/hscript-validate ${CMAKE_SOURCE_DIR}/tests/fixtures/0001-basic.installfile)
ENDIF(VALGRIND)

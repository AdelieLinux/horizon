set(VALIDATE_SRCS
	validator.cc
)
add_executable(hscript-validate ${VALIDATE_SRCS})
target_link_libraries(hscript-validate hscript)

install(TARGETS hscript-validate DESTINATION bin)

add_test(NAME "RSpecValidator"
    COMMAND rspec spec/validator.rb
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
set_property(TEST "RSpecValidator"
    PROPERTY ENVIRONMENT "PATH=$PATH:${CMAKE_CURRENT_BINARY_DIR}/tools/hscript-validate")
IF(VALGRIND)
add_test(NAME "ValgrindValidator"
    COMMAND valgrind ${CMAKE_CURRENT_BINARY_DIR}/tools/hscript-validate/hscript-validate tests/fixtures/0001-basic.installfile)
ENDIF(VALGRIND)

set(VALIDATE_SRCS
	validator.cc
)
add_executable(hscript-validate ${VALIDATE_SRCS})
target_link_libraries(hscript-validate hscript ${Boost_LIBRARIES})

install(TARGETS hscript-validate DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/validator.1 DESTINATION share/man/man1 RENAME hscript-validate.1)

IF(RSPEC_EXECUTABLE)
add_test(NAME "RSpecValidator"
    COMMAND ${RSPEC_EXECUTABLE} spec/validator_spec.rb
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
set_property(TEST "RSpecValidator"
    PROPERTY ENVIRONMENT "PATH=$ENV{PATH}:${CMAKE_CURRENT_BINARY_DIR}")
ENDIF(RSPEC_EXECUTABLE)

IF(VALGRIND)
add_test(NAME "ValgrindValidator"
    COMMAND ${VALGRIND_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/hscript-validate ${CMAKE_SOURCE_DIR}/tests/fixtures/0001-basic.installfile)
ENDIF(VALGRIND)

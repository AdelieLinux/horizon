set(OWNER_SRCS
        owner.c
)
add_executable(hscript-printowner ${OWNER_SRCS})

install(TARGETS hscript-printowner DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/owner.1 DESTINATION share/man/man1 RENAME hscript-printowner.1)

IF(RSPEC_EXECUTABLE)
add_test(NAME "RSpecOwner"
    COMMAND ${RSPEC_EXECUTABLE} spec/owner_spec.rb
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
set_property(TEST "RSpecOwner"
    PROPERTY ENVIRONMENT "PATH=$ENV{PATH}:${CMAKE_CURRENT_BINARY_DIR}")
ENDIF(RSPEC_EXECUTABLE)

IF(VALGRIND)
add_test(NAME "ValgrindOwner"
    COMMAND ${VALGRIND_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/hscript-printowner ${CMAKE_SOURCE_DIR})
ENDIF(VALGRIND)

pkg_check_modules(CURL libcurl)

set(FETCH_SRCS
        fetch.cc
)
add_executable(hscript-fetch ${FETCH_SRCS})

IF(CURL_FOUND)
    add_definitions(-DHAVE_LIBCURL)
    target_link_libraries(hscript-fetch ${CURL_LIBRARIES})
ENDIF(CURL_FOUND)

install(TARGETS hscript-fetch DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/fetch.1 DESTINATION share/man/man1 RENAME hscript-fetch.1)

#IF(RSPEC_EXECUTABLE)
#add_test(NAME "RSpecFetch"
#    COMMAND ${RSPEC_EXECUTABLE} spec/fetch_spec.rb
#    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
#set_property(TEST "RSpecFetch"
#    PROPERTY ENVIRONMENT "PATH=$ENV{PATH}:${CMAKE_CURRENT_BINARY_DIR}")
#ENDIF(RSPEC_EXECUTABLE)

IF(VALGRIND)
add_test(NAME "ValgrindFetch"
    COMMAND ${VALGRIND_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/hscript-fetch https://horizon.adelielinux.org/example.installfile)
ENDIF(VALGRIND)

set(JSONCONV_SRCS
        jsonconv.cc
)
add_executable(hscript-fromjson ${JSONCONV_SRCS})
target_link_libraries(hscript-fromjson hscript ${FS_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS hscript-fromjson DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/jsonconv.1 DESTINATION share/man/man1 RENAME hscript-fromjson.1)

if("cxx_std_17" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set_property(TARGET hscript-fromjson PROPERTY CXX_STANDARD 17)
endif()

#IF(RSPEC_EXECUTABLE)
#add_test(NAME "RSpecJSONConv"
#    COMMAND ${RSPEC_EXECUTABLE} spec/jsonconv_spec.rb
#    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
#set_property(TEST "RSpecJSONConv"
#    PROPERTY ENVIRONMENT "PATH=$ENV{PATH}:${CMAKE_CURRENT_BINARY_DIR}")
#ENDIF(RSPEC_EXECUTABLE)
#
#IF(VALGRIND)
#add_test(NAME "ValgrindJSONConv"
#    COMMAND ${VALGRIND_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/hscript-fromjson ${CMAKE_SOURCE_DIR}/tests/fixtures/json/0001-basic.json)
#ENDIF(VALGRIND)

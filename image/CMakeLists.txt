find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(backends)

set(IMG_SRCS
        creator.cc
)
add_executable(hscript-image ${IMG_SRCS})
target_link_libraries(hscript-image hscript hi-backends ${Boost_LIBRARIES})
install(TARGETS hscript-image DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/creator.1 DESTINATION share/man/man1 RENAME hscript-image.1)

add_subdirectory(backends)

set(IMG_SRCS
        creator.cc
)
add_executable(hscript-image ${IMG_SRCS})

if("cxx_std_17" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    find_package(Boost REQUIRED COMPONENTS program_options)
    set_property(TARGET hscript-image PROPERTY CXX_STANDARD 17)
else()
    find_package(Boost REQUIRED COMPONENTS filesystem program_options)
endif()

include_directories(${Boost_INCLUDE_DIR})

target_link_libraries(hscript-image hscript hi-backends ${Boost_LIBRARIES})
install(TARGETS hscript-image DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/creator.1 DESTINATION share/man/man1 RENAME hscript-image.1)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/iso-share/ DESTINATION share/horizon/iso)

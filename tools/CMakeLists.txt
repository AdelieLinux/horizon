find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(hscript-validate)
add_subdirectory(hscript-simulate)

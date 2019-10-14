cmake_minimum_required(VERSION 3.4)

include(FindPkgConfig)

project(Horizon
	LANGUAGES CXX
	VERSION 0.1.0)

pkg_check_modules(LIBUDEV REQUIRED libudev)
pkg_check_modules(PARTED REQUIRED libparted)
find_library(BCNM_LIBRARY REQUIRED wpactrl PATH_SUFFIXES bcnm)

option(BUILD_TOOLS "Enable building of tools (Validator, Simulator, etc)" ON)
option(COVERAGE "Build for code coverage tests (slow)" OFF)
option(VALGRIND "Run Valgrind during test phase" OFF)

IF(COVERAGE)
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage")
ENDIF(COVERAGE)

include_directories(.)

add_subdirectory(hscript)
IF(BUILD_TOOLS)
    add_subdirectory(tools)
ENDIF(BUILD_TOOLS)

enable_testing()
IF(BUILD_TOOLS)
    add_test(NAME "RSpecValidator"
        COMMAND rspec spec/validator.rb
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
    set_property(TEST "RSpecValidator"
        PROPERTY ENVIRONMENT "PATH=$PATH:${CMAKE_CURRENT_BINARY_DIR}/tools/hscript-validate")
    add_test(NAME "RSpecSimulator"
        COMMAND rspec spec/simulator.rb
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
    set_property(TEST "RSpecSimulator"
        PROPERTY ENVIRONMENT "PATH=$PATH:${CMAKE_CURRENT_BINARY_DIR}/tools/hscript-simulate")
    IF(VALGRIND)
    add_test(NAME "ValgrindValidator"
        COMMAND valgrind ${CMAKE_CURRENT_BINARY_DIR}/tools/hscript-validate/hscript-validate tests/fixtures/0001-basic.installfile)
    add_test(NAME "ValgrindSimulator"
        COMMAND valgrind ${CMAKE_CURRENT_BINARY_DIR}/tools/hscript-simulate/hscript-simulate tests/fixtures/0001-basic.installfile)
    ENDIF(VALGRIND)
ENDIF(BUILD_TOOLS)

cmake_minimum_required(VERSION 3.4)

include(FindPkgConfig)
include(CheckIncludeFiles)

project(Horizon
	LANGUAGES CXX
	VERSION 0.1.0)

option(BUILD_TOOLS "Enable building of tools (Validator, Simulator, etc)" ON)
option(COVERAGE "Build for code coverage tests (slow)" OFF)
option(VALGRIND "Run Valgrind during test phase" OFF)
option(INSTALL "Build Installation Environment support (Linux only)" ON)

option(UNSUPPORTED_NONFREE_FIRMWARE "Support loading and installation of non-libre firmware (DANGEROUS)" OFF)
mark_as_advanced(FORCE UNSUPPORTED_NONFREE_FIRMWARE)

IF(UNSUPPORTED_NONFREE_FIRMWARE)
    add_definitions(-DNON_LIBRE_FIRMWARE)
ENDIF(UNSUPPORTED_NONFREE_FIRMWARE)

check_include_files(linux/wireless.h HAVE_LINUX_WIRELESS_H)
IF(NOT HAVE_LINUX_WIRELESS_H)
SET(INSTALL OFF)
ENDIF(NOT HAVE_LINUX_WIRELESS_H)

IF(INSTALL)
    add_definitions(-DHAS_INSTALL_ENV)
    pkg_check_modules(BLKID REQUIRED blkid)
    pkg_check_modules(LIBUDEV REQUIRED libudev)
    pkg_check_modules(PARTED REQUIRED libparted)
    find_library(BCNM_LIBRARY REQUIRED wpactrl PATH_SUFFIXES bcnm)
ENDIF(INSTALL)

IF(COVERAGE)
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage")
ENDIF(COVERAGE)

include_directories(.)
enable_testing()

add_subdirectory(hscript)
IF(BUILD_TOOLS)
    add_subdirectory(tools)
ENDIF(BUILD_TOOLS)

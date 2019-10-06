cmake_minimum_required(VERSION 3.4)

include(FindPkgConfig)

project(Horizon
	LANGUAGES CXX
	VERSION 0.1.0)

pkg_check_modules(LIBUDEV REQUIRED libudev)
pkg_check_modules(PARTED REQUIRED libparted)
find_library(BCNM_LIBRARY REQUIRED wpactrl PATH_SUFFIXES bcnm)

add_subdirectory(hscript)
add_subdirectory(tools)

cmake_minimum_required(VERSION 3.4)

include(FindPkgConfig)

project(Horizon
	LANGUAGES CXX
	VERSION 0.1.0)

pkg_check_modules(LIBUDEV REQUIRED libudev)
pkg_check_modules(PARTED REQUIRED libparted)
find_library(BCNM_LIBRARY REQUIRED wpactrl PATH_SUFFIXES bcnm)

set(HSCRIPT_SOURCE
	hscript/script.cc
)

set(HSCRIPT_INCLUDE
        hscript/script.hh
        hscript/key.hh
)

add_library(hscript SHARED ${HSCRIPT_SOURCE})
target_compile_features(hscript PRIVATE cxx_nullptr)
target_compile_features(hscript PUBLIC cxx_unicode_literals)

install(TARGETS hscript DESTINATION lib)
install(FILES ${HSCRIPT_INCLUDE} DESTINATION include/hscript)

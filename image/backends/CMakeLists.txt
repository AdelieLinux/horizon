set(BACKEND_SRCS
        basic.cc
)

set(BACKEND_LIBS "")

find_package(LibArchive)

if(LibArchive_FOUND)
    list(APPEND BACKEND_SRCS tar.cc)
    list(APPEND BACKEND_LIBS ${LibArchive_LIBRARIES})
endif()

if(BUILD_ISO)
    list(APPEND BACKEND_SRCS iso.cc)
endif(BUILD_ISO)

add_library(hi-backends ${BACKEND_SRCS})
target_link_libraries(hi-backends ${BACKEND_LIBS})
install(TARGETS hi-backends DESTINATION lib)

if("cxx_std_17" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set_property(TARGET hi-backends PROPERTY CXX_STANDARD 17)
endif()

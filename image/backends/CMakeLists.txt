set(BACKEND_SRCS
        basic.cc
)

set(BACKEND_LIBS "")

find_package(LibArchive)

if(LibArchive_FOUND)
    list(APPEND BACKEND_SRCS tar.cc)
    list(APPEND BACKEND_LIBS ${LibArchive_LIBRARIES})
endif()

add_library(hi-backends ${BACKEND_SRCS})
target_link_libraries(hi-backends ${BACKEND_LIBS})
install(TARGETS hi-backends DESTINATION lib)

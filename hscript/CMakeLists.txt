set(HSCRIPT_SOURCE
	script.cc
        disk.cc
        key.cc
        meta.cc
        network.cc
        user.cc
)

set(HSCRIPT_INCLUDE
        script.hh
)

add_library(hscript ${HSCRIPT_SOURCE})
target_compile_features(hscript PRIVATE cxx_nullptr)
target_compile_features(hscript PUBLIC cxx_unicode_literals)
if("cxx_std_17" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set_property(TARGET hscript PROPERTY CXX_STANDARD 17)
    SET(FS_LIBRARY stdc++fs)
    add_definitions(-DFS_IS_STDCXX)
ELSE()
    find_package(Boost REQUIRED COMPONENTS filesystem)
    SET(FS_LIBRARY ${Boost_FILESYSTEM_LIBRARY})
    add_definitions(-DFS_IS_BOOST)
ENDIF()
target_link_libraries(hscript ${BLKID_LIBRARIES} ${FS_LIBRARY} ${LIBUDEV_LIBRARIES} ${PARTED_LIBRARIES})

install(TARGETS hscript DESTINATION lib)
install(FILES ${HSCRIPT_INCLUDE} DESTINATION include/hscript)

pkg_check_modules(CURL libcurl)

set(HSCRIPT_SOURCE
	script.cc
        script_v.cc
        script_e.cc
        disk.cc
        disk_lvm.cc
        key.cc
        meta.cc
        network.cc
        user.cc
        util.cc
)

set(HSCRIPT_INCLUDE
        script.hh
)

add_library(hscript ${HSCRIPT_SOURCE})
target_compile_features(hscript PRIVATE cxx_nullptr)
target_compile_features(hscript PUBLIC cxx_unicode_literals)

if("cxx_std_17" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set_property(TARGET hscript PROPERTY CXX_STANDARD 17)
endif()

target_link_libraries(hscript ${BLKID_LIBRARIES} ${FS_LIBRARY} ${LIBUDEV_LIBRARIES} ${PARTED_LIBRARIES})

IF(CURL_FOUND)
    add_definitions(-DHAVE_LIBCURL)
    target_link_libraries(hscript ${CURL_LIBRARIES})
ENDIF(CURL_FOUND)

install(TARGETS hscript DESTINATION lib)
install(FILES ${HSCRIPT_INCLUDE} DESTINATION include/hscript)

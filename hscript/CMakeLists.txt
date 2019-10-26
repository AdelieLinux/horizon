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
target_link_libraries(hscript ${BLKID_LIBRARIES} ${LIBUDEV_LIBRARIES} ${PARTED_LIBRARIES})

install(TARGETS hscript DESTINATION lib)
install(FILES ${HSCRIPT_INCLUDE} DESTINATION include/hscript)

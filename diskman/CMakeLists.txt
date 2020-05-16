pkg_check_modules(FDISK REQUIRED fdisk)
find_package(Boost REQUIRED)

set(DISKMAN_SOURCE
        diskman.cc
        disk.cc
        lvmhelp.cc
        partition.cc
)

set(DISKMAN_INCLUDE
        diskman.hh
        disk.hh
        partition.hh
)

include_directories(${Boost_INCLUDE_DIRS})
add_library(diskman ${DISKMAN_SOURCE})
target_link_libraries(diskman ${FDISK_LIBRARIES} ${LIBUDEV_LIBRARIES})

install(TARGETS diskman DESTINATION lib)
install(FILES ${DISKMAN_INCLUDE} DESTINATION include/diskman)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/man/ DESTINATION share/man/man3)

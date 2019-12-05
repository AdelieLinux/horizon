set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(UI_SOURCES
    horizonwizard.cc
    horizonwizardpage.cc
    horizonhelpwindow.cc
    main.cc
    ${CMAKE_SOURCE_DIR}/3rdparty/Section.cpp
    useraccountwidget.cc
    crypt_sha512.c

    intropage.cc
    inputpage.cc
    networkingpage.cc
    networkifacepage.cc
    netsimplewifipage.cc
    netdhcppage.cc
    datetimepage.cc
    hostnamepage.cc
    pkgsimple.cc
    bootpage.cc
    rootpwpage.cc
    accountpage.cc

    horizon.qrc)

IF(UNSUPPORTED_NONFREE_FIRMWARE)
    LIST(APPEND UI_SOURCES firmwarepage.cc)
ENDIF(UNSUPPORTED_NONFREE_FIRMWARE)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
add_executable(horizon-qt5 ${UI_SOURCES})
target_link_libraries(horizon-qt5 Qt5::Network Qt5::Widgets)

install(TARGETS horizon-qt5 DESTINATION bin)

IF(INSTALL)
    pkg_check_modules(XKBFile REQUIRED xkbfile)
    pkg_check_modules(LIBX11 REQUIRED x11)
    pkg_check_modules(LibCap REQUIRED libcap)
    target_link_libraries(horizon-qt5 ${BCNM} ${SKARNET} ${LIBUDEV_LIBRARIES} ${LibCap_LIBRARIES} ${LIBX11_LIBRARIES} ${XKBFile_LIBRARIES})
    # add_executable(horizon-run-qt5 ${RUN_QT_SOURCES})
ENDIF(INSTALL)

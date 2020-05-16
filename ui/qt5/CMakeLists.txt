set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(UI_SOURCES
    horizonwizard.cc
    horizonwizardpage.cc
    horizonhelpwindow.cc
    main.cc
    ${CMAKE_SOURCE_DIR}/3rdparty/Section.cpp
    stepprogresswidget.cc
    subnetbox.cc
    useraccountwidget.cc
    avatardialog.cc
    crypt_sha512.c

    intropage.cc
    inputpage.cc
    partitiondiskpage.cc
    partitionchoicepage.cc
    partitionmanualpage.cc
    networkingpage.cc
    networkifacepage.cc
    netsimplewifipage.cc
    netdhcppage.cc
    netmanualpage.cc
    datetimepage.cc
    hostnamepage.cc
    pkgsimple.cc
    pkgmodel.cc
    pkgcustom.cc
    pkgdefaults.cc
    bootpage.cc
    rootpwpage.cc
    accountpage.cc

    horizon.qrc)

set(RUN_QT_SOURCES
    horizonhelpwindow.cc
    horizonwizardpage.cc
    stepprogresswidget.cc
    runner/main.cc
    runner/executorwizard.cc
    runner/executepage.cc
    runner/errorpage.cc
    runner/finishedpage.cc

    horizon.qrc)

IF(INSTALL)
    LIST(APPEND UI_SOURCES
        commitpage.cc
        partitionprobe.cc
        partitionpage.cc)
ELSE(INSTALL)
    LIST(APPEND UI_SOURCES writeoutpage.cc)
ENDIF(INSTALL)

IF(UNSUPPORTED_NONFREE_FIRMWARE)
    LIST(APPEND UI_SOURCES firmwarepage.cc)
ENDIF(UNSUPPORTED_NONFREE_FIRMWARE)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
add_executable(horizon-qt5 ${UI_SOURCES})
target_link_libraries(horizon-qt5 Qt5::Network Qt5::Widgets crypto)

IF("cxx_std_17" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set_property(TARGET horizon-qt5 PROPERTY CXX_STANDARD 17)
ENDIF()

install(TARGETS horizon-qt5 DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/horizon-qt5.1 DESTINATION share/man/man1)

IF(INSTALL)
    pkg_check_modules(XKBFile REQUIRED xkbfile)
    pkg_check_modules(LIBX11 REQUIRED x11)
    pkg_check_modules(LibCap REQUIRED libcap)
    target_link_libraries(horizon-qt5 ${BCNM} ${SKARNET} ${LIBUDEV_LIBRARIES} ${LibCap_LIBRARIES} ${LIBX11_LIBRARIES} ${XKBFile_LIBRARIES} diskman)
    add_executable(horizon-run-qt5 ${RUN_QT_SOURCES})
    target_link_libraries(horizon-run-qt5 Qt5::Widgets)
    target_compile_definitions(horizon-run-qt5 PRIVATE IN_RUNNER)
    IF("cxx_std_17" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
        set_property(TARGET horizon-run-qt5 PROPERTY CXX_STANDARD 17)
    ENDIF()
    install(TARGETS horizon-run-qt5 DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/horizon-run-qt5.8 DESTINATION share/man/man8)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/horizon-ppc64-detect DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/horizon-ppc64-detect.1 DESTINATION share/man/man1)
ENDIF(INSTALL)

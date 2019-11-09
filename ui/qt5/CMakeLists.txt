set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(UI_SOURCES
    horizonwizard.cc
    horizonwizardpage.cc
    horizonhelpwindow.cc
    main.cc

    intropage.cc
    networkingpage.cc

    horizon.qrc)
add_executable(horizon-qt5 ${UI_SOURCES})
target_link_libraries(horizon-qt5 Qt5::Widgets)

install(TARGETS horizon-qt5 DESTINATION bin)

IF(INSTALL)
    # add_executable(horizon-run-qt5 ${RUN_QT_SOURCES})
ENDIF(INSTALL)

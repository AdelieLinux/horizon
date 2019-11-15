find_package(Qt5 COMPONENTS Network Widgets)

IF(Qt5_FOUND)
    option(BUILD_QT "Build the Qt 5 user interface" ON)
    IF(BUILD_QT)
        add_subdirectory(qt5)
    ENDIF(BUILD_QT)
ENDIF(Qt5_FOUND)

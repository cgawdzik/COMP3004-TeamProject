cmake_minimum_required(VERSION 3.16)

project(COMP3004-FinalProject VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Install QtCharts
# sudo apt-get install libqt5charts5-dev

# Find Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    bolus_manager.h
    bolus_manager.cpp
    cgm_simulator.h
    cgm_simulator.cpp
    control_iq_manager.h
    control_iq_manager.cpp
    glucose_graph_widget.h
    glucose_graph_widget.cpp
    profile.h
    profile.cpp
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(COMP3004-FinalProject
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(COMP3004-FinalProject SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(COMP3004-FinalProject
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Common target properties
target_link_libraries(COMP3004-FinalProject PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
)

# macOS/iOS bundle settings
if(QT_VERSION VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.COMP3004-FinalProject)
endif()

set_target_properties(COMP3004-FinalProject PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Installation
include(GNUInstallDirs)
install(TARGETS COMP3004-FinalProject
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Qt6 finalization
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(COMP3004-FinalProject)
endif()

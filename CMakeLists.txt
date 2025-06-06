cmake_minimum_required(VERSION 3.10)

project(nemofingerprint
    VERSION 0.1
    DESCRIPTION "Provides QML plugin for fprintd")

set(CMAKE_AUTOMOC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(FeatureSummary)
include(GNUInstallDirs)

set(QT_MIN_VERSION "5.12.0")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Qml Quick DBus LinguistTools )
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Qml Quick DBus LinguistTools REQUIRED)


if(${QT_VERSION} VERSION_LESS "5.15.0")
    macro(qt_add_translation)
        qt5_add_translation(${ARGN})
    endmacro()
    macro(qt_add_resources)
        qt5_add_resources(${ARGN})
    endmacro()
    macro(qt_add_dbus_interface)
        qt5_add_dbus_interface(${ARGN})
    endmacro()
endif()

add_subdirectory(src)

# Translations
file(GLOB TS_FILES translations/*.ts)
qt_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES})
add_dependencies(nemofingerprint translations)

install(FILES ${QM_FILES}
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/nemofingerprint/translations)

feature_summary(WHAT PACKAGES_FOUND PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

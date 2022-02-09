cmake_minimum_required(VERSION 3.1.0)

project(nemofingerprint
    VERSION 0.1
    DESCRIPTION "Provides QML plugin for fprintd")

set(CMAKE_AUTOMOC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(FeatureSummary)
include(GNUInstallDirs)

set(QT_MIN_VERSION "5.12.0")
find_package(Qt5 COMPONENTS Core Qml Quick DBus LinguistTools REQUIRED)

add_subdirectory(src)

# Translations
file(GLOB TS_FILES translations/*.ts)
qt5_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES})
add_dependencies(nemofingerprint translations)

install(FILES ${QM_FILES}
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/nemofingerprint/translations)

feature_summary(WHAT PACKAGES_FOUND PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

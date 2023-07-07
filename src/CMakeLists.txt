### Sets QT_INSTALL_QML to the directory where QML Plugins should be installed
function(FindQtInstallQml)
    if (QT_VERSION_MAJOR EQUAL 5)
        find_program(QMAKE NAMES qmake-qt5 qmake)
    elseif (QT_VERSION_MAJOR EQUAL 6)
        find_program(QMAKE NAMES qmake-qt6 qmake6)
    else()
        message(FATAL_ERROR "incorrect QT_VERSION_MAJOR")
    endif ()

    if(NOT QMAKE)
        message(FATAL_ERROR "qmake not found")
    endif()
    execute_process(
        COMMAND ${QMAKE} -query QT_INSTALL_QML
        OUTPUT_VARIABLE PROC_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(QT_INSTALL_QML ${PROC_RESULT} PARENT_SCOPE)
endfunction()

set_source_files_properties(net.reactivated.Fprint.Device.xml
                        PROPERTIES NO_NAMESPACE TRUE)

set_source_files_properties(net.reactivated.Fprint.Manager.xml
                        PROPERTIES NO_NAMESPACE TRUE)

set(nemo_fingerptint_SRCS
    plugin.cpp
    finger.cpp
    fingerprintdevice.cpp
    fingerprintmodel.cpp)

set(HEADERS
    finger.h
    fingerprintdevice.h
    fingerprintmodel.h)

qt_add_dbus_interface(nemo_fingerptint_SRCS
    net.reactivated.Fprint.Device.xml
    fprint_device_interface
)

qt_add_dbus_interface(nemo_fingerptint_SRCS
    net.reactivated.Fprint.Manager.xml
    fprint_manager_interface
)

add_library(nemofingerprint SHARED ${nemo_fingerptint_SRCS} ${HEADERS})

target_link_libraries(nemofingerprint
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::DBus)

FindQtInstallQml()

install(FILES qmldir DESTINATION ${QT_INSTALL_QML}/org/nemomobile/fingerprint/)
install(TARGETS nemofingerprint DESTINATION ${QT_INSTALL_QML}/org/nemomobile/fingerprint/)

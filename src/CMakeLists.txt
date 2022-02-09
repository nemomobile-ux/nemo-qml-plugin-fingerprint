### Sets QT_INSTALL_QML to the directory where QML Plugins should be installed
function(FindQtInstallQml)
    find_program(QMAKE NAMES qmake-qt5 qmake)
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

qt5_add_dbus_interface(nemo_fingerptint_SRCS
    net.reactivated.Fprint.Device.xml
    fprint_device_interface
)

qt5_add_dbus_interface(nemo_fingerptint_SRCS
    net.reactivated.Fprint.Manager.xml
    fprint_manager_interface
)

add_library(nemofingerprint SHARED ${nemo_fingerptint_SRCS} ${HEADERS})

target_link_libraries(nemofingerprint
    Qt5::Core
    Qt5::Qml 
    Qt5::Quick
    Qt5::DBus)

FindQtInstallQml()

install(FILES qmldir DESTINATION ${QT_INSTALL_QML}/org/nemomobile/fingerprint/)
install(TARGETS nemofingerprint DESTINATION ${QT_INSTALL_QML}/org/nemomobile/fingerprint/)

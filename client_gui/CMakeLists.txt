cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Boost REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOGEN_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/autogen)

set(CLIENT_GUI_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        ui/mainwindow.ui
        resource.qrc
        src/client_gui_event_handler.h
        src/filters_dialog.h
        src/filters_dialog.cpp
        ui/filters_dialog.ui
)

qt_add_executable(client_gui
    MANUAL_FINALIZATION
    ${CLIENT_GUI_SOURCES}
)

target_include_directories(client_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_AUTOGEN_BUILD_DIR}

    ${CMAKE_SOURCE_DIR}/client_lib/src
    ${CMAKE_SOURCE_DIR}/common/push/
    ${CMAKE_SOURCE_DIR}/network/client/include/
)

set_property(TARGET client_gui PROPERTY AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(CMAKE_AUTOUIC_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)

# target_link_libraries(client_gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(client_gui
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Boost::headers
        client_lib
)

if(WIN32)
    target_link_libraries(client_gui
        PRIVATE
            ws2_32
            mswsock
    )
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.client_gui)
endif()

# Свойства приложения
set_target_properties(client_gui PROPERTIES # Настраивает поведение бинарника на платформах
    ${BUNDLE_ID_OPTION}
    WIN32_EXECUTABLE TRUE # отключает консольное окно и приложение становится GUI
    MACOSX_BUNDLE TRUE # создаётся .app bundle
)

include(GNUInstallDirs)
install(TARGETS client_gui
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(client_gui)
	
endif()




set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Quick Location Positioning Multimedia REQUIRED)

set(PLAYER_SRCS
    main.cc
    declarative/declarative.cc
    declarative/declarative.hh
    declarative/QmlAppWindow.cc
    declarative/QmlAppWindow.hh
    declarative/QmlLibrary.cc
    declarative/QmlLibrary.hh
    declarative/QmlObject.hh
    declarative/QmlObject.cc
    declarative/QmlTrip.cc
    declarative/QmlTrip.hh
    qml.qrc
    )

if (WIN32)
    list(APPEND PLAYER_SRCS
        qt5-player.rc
        qt5-player.ico
        )
endif()

add_executable(qt5-player ${PLAYER_SRCS})
set_target_properties(qt5-player PROPERTIES OUTPUT_NAME "${PROJECT_NAME}-player" FOLDER ${FOLDER_APPS})
target_compile_definitions(qt5-player PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(qt5-player PRIVATE Qt5::Core Qt5::Quick Qt5::Location Qt5::Positioning Qt5::Multimedia mgps)
target_include_directories(qt5-player PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

include(dir_filters)
dir_filters(PLAYER_SRCS)

include(application)
bindir(qt5-player)

set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

install(TARGETS qt5-player)
if (UNIX)
    install(FILES dashcam-gps-player.desktop DESTINATION share/applications)
    install(FILES img/icon.png DESTINATION share/icons/hicolor/256x256/apps RENAME dashcam-gps-player.png)
endif()
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
    declarative/QmlDrive.cc
    declarative/QmlDrive.hh
    declarative/QmlObject.hh
    declarative/QmlObject.cc
    qml.qrc
    )

if (WIN32)
    list(APPEND PLAYER_SRCS
        mgps-player.rc
        )
endif()

add_executable(qt5-mgps-player ${PLAYER_SRCS})
set_target_properties(qt5-mgps-player PROPERTIES OUTPUT_NAME "mgps-player")
target_compile_definitions(qt5-mgps-player PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(qt5-mgps-player PRIVATE Qt5::Core Qt5::Quick Qt5::Location Qt5::Positioning Qt5::Multimedia mgps-70mai)

include(dir_filters)
dir_filters(PLAYER_SRCS)
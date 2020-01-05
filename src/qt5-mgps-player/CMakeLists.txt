set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Quick Location Positioning REQUIRED)

set(PLAYER_SRCS
    main.cc
    appwindow.cc
    appwindow.hh
    qml_trip.cc
    qml_trip.hh
    qml.qrc
    )

if (WIN32)
    list(APPEND PLAYER_SRCS
        mgps-player.rc
        )
endif()

add_executable(mgps-player ${PLAYER_SRCS})
target_compile_definitions(mgps-player PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(mgps-player PRIVATE Qt5::Core Qt5::Quick Qt5::Location Qt5::Positioning mgps-70mai)

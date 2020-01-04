set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Quick Location Positioning REQUIRED)

add_executable(mgps-player
    main.cc
    qml/main.qml
    qml/MoviePlayer.qml
    qml/RouteViewer.qml
    qml_trip.cc
    qml_trip.hh
    qml.qrc
    )
target_compile_definitions(mgps-player PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(mgps-player PRIVATE Qt5::Core Qt5::Quick Qt5::Location Qt5::Positioning mgps-70mai)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Quick Location REQUIRED)

add_executable(mgps-player main.cc main.qml qml.qrc)
target_compile_definitions(mgps-player PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(mgps-player PRIVATE Qt5::Core Qt5::Quick Qt5::Location mgps-70mai)

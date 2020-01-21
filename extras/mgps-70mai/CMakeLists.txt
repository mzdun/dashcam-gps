include(warnings)
include(dir_filters)

set(PLUGIN_SOURCES
    include/mgps-70mai/70mai.hh
    include/mgps-70mai/plugin.hh
    src/70mai.cc
    src/plugin.cc
)
add_library(mgps-70mai STATIC ${PLUGIN_SOURCES})

target_include_directories(mgps-70mai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mgps-70mai mgps ctre::ctre)
set_target_properties(mgps-70mai PROPERTIES
    FOLDER ${FOLDER_EXTRAS}
    POSITION_INDEPENDENT_CODE ON)

dir_filters(PLUGIN_SOURCES)
